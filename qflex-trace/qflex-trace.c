/*
 * Copyright (C) 2021, Mahmoud Mandour <ma.mandourr@gmail.com>
 *
 * License: GNU GPL, version 2 or later.
 *   See the COPYING file in the top-level directory.
 */

#include <inttypes.h>
#include <stdio.h>
#include <glib.h>

#include <qemu-plugin.h>
#include "qflex-api.h"
#include "qflex-trace-decoder.h"

#define STRTOLL(x) g_ascii_strtoll(x, NULL, 10)
#define MAX_CPUS 128

QEMU_PLUGIN_EXPORT int qemu_plugin_version = QEMU_PLUGIN_VERSION;

static int instruction_count = 0;

static enum qemu_plugin_mem_rw rw = QEMU_PLUGIN_MEM_RW;

static GHashTable *miss_ht;

static GMutex hashtable_lock;
//static GRand *rng;

static QEMU_TO_QFLEX_CALLBACKS_t *qflex_callbacks = NULL;

static int cpuFirst = 0;
static int cpuLast = MAX_CPUS;

typedef struct {
    uint64_t gVA_pc;
    uint64_t gPA_pc;
    uint32_t insn;
    int byte_size;
    bool is_user;
    bool has_mem;
    MemTraceParams meta_mem;
    bool has_br;
    BranchTraceParams meta_br;
} InsnData;

static memory_transaction_t last_insn_fetch[MAX_CPUS];

static void mem_callback(InsnData *meta, unsigned int vcpu_index, bool is_store,
                         bool is_io, uint64_t haddr, uint64_t vaddr) {
    memory_transaction_t mem_trans;
    mem_trans.s.pc = meta->gVA_pc;
    mem_trans.s.logical_address = vaddr;
    mem_trans.s.physical_address = qemu_plugin_get_gpaddr(vaddr, is_store ? DATA_STORE : DATA_LOAD);
    mem_trans.s.type = is_store ? QEMU_Trans_Store : QEMU_Trans_Load;
    mem_trans.s.size = meta->meta_mem.size;
    mem_trans.s.atomic = meta->meta_mem.is_atomic;
    mem_trans.arm_specific.user = meta->is_user;
    mem_trans.io = is_io;
    qflex_callbacks->trace_mem(vcpu_index, &mem_trans);
}

static void insn_callback(InsnData *meta, unsigned int vcpu_index) 
{
    memory_transaction_t mem_trans;
    mem_trans.s.pc = meta->gVA_pc;
    mem_trans.s.physical_address = meta->gPA_pc;
    mem_trans.s.target_address = -1;
    mem_trans.s.type = QEMU_Trans_Instr_Fetch;
    mem_trans.s.size = meta->byte_size;
    mem_trans.s.branch_type = meta->has_br ? meta->meta_br.branch_type : QEMU_Non_Branch;
    mem_trans.s.annul = 0; // Deprecated?
    mem_trans.s.opcode = meta->insn;
    mem_trans.io = false;
    mem_trans.arm_specific.user = meta->is_user;
    if (last_insn_fetch[vcpu_index].s.pc != 0) {
        last_insn_fetch[vcpu_index].s.target_address = meta->gVA_pc;
        if (qemu_plugin_get_gpaddr(last_insn_fetch[vcpu_index].s.pc, INST_FETCH) != -1) {
            // printf("PC: %016lx, PADDR: %016lx, type:%i, opcode: %x\n", last_insn_fetch[vcpu_index].s.pc, paddr, last_insn_fetch[vcpu_index].s.branch_type, last_insn_fetch[vcpu_index].s.opcode);
            qflex_callbacks->trace_mem(vcpu_index, &last_insn_fetch[vcpu_index]);
        } else {
            g_autoptr(GString) outs = g_string_new("Failed to translate PC addr:");
            g_string_append_printf(outs, "%016lx", meta->gVA_pc);
            qemu_plugin_outs(outs->str);
        }
    } else {
        qemu_plugin_outs("Not sending instruction, detected PC 0\n");
    }

    last_insn_fetch[vcpu_index] = mem_trans;
    instruction_count++;
    if (instruction_count%1000000 == 0){
        printf("[Instruction Count] : %d Million Instructions\n", instruction_count/1000000);
    }
}

static void vcpu_mem_access(unsigned int vcpu_index, qemu_plugin_meminfo_t info,
                            uint64_t vaddr, void *userdata)
{
    if (!(cpuFirst <= vcpu_index && vcpu_index <= cpuLast)) return;
 
    InsnData *insn = ((InsnData *) userdata);
    struct qemu_plugin_hwaddr *hwaddr;
    bool is_io = false;
    bool is_store = qemu_plugin_mem_is_store(info);

    g_assert(insn->has_mem);
    if (is_store) {
        g_assert(insn->meta_mem.is_store);
    } else {
        g_assert(insn->meta_mem.is_load);
    }

    hwaddr = qemu_plugin_get_hwaddr(info, vaddr);
    is_io = hwaddr && qemu_plugin_hwaddr_is_io(hwaddr);
    uint64_t haddr = qemu_plugin_hwaddr_phys_addr(hwaddr);
    mem_callback(insn, vcpu_index, is_store, is_io, haddr, vaddr);
}

static void vcpu_insn_exec(unsigned int vcpu_index, void *userdata)
{
    if (!(cpuFirst <= vcpu_index && vcpu_index <= cpuLast)) return;
 
    InsnData *insn = ((InsnData *) userdata);
    insn_callback(insn, vcpu_index);
}

static void vcpu_tb_trans(qemu_plugin_id_t id, struct qemu_plugin_tb *tb)
{
    size_t n_insns;
    size_t i;
    InsnData *data;

    n_insns = qemu_plugin_tb_n_insns(tb);
    for (i = 0; i < n_insns; i++) {
        struct qemu_plugin_insn *insn = qemu_plugin_tb_get_insn(tb, i);
        uint64_t gVA_pc = (uint64_t) qemu_plugin_insn_vaddr(insn);
        uint64_t hVA_pc = (uint64_t) qemu_plugin_insn_haddr(insn);
        uint64_t gPA_pc = qemu_plugin_get_gpaddr(gVA_pc, INST_FETCH);
        assert(gPA_pc != -1);

        /*
         * Instructions might get translated multiple times, we do not create
         * new entries for those instructions. Instead, we fetch the same
         * entry from the hash table and register it for the callback again.
         */
        g_mutex_lock(&hashtable_lock);
        data = g_hash_table_lookup(miss_ht, GUINT_TO_POINTER(hVA_pc));
        if (data == NULL) {
            data = g_new0(InsnData, 1);
            data->gPA_pc = gPA_pc;
            data->gVA_pc = gVA_pc;
            data->insn = *(uint32_t *) hVA_pc;
            data->is_user = qemu_plugin_is_userland(insn);
            data->byte_size = qemu_plugin_insn_size(insn);
            data->has_mem = aarch64_insn_get_params_mem(&data->meta_mem, data->insn);
            data->has_br = aarch64_insn_get_params_branch(&data->meta_br, data->insn);
            //printf("libqflex, pc: %lX, inst: %X\n", data->gVA_pc, data->insn);
            g_hash_table_insert(miss_ht, GUINT_TO_POINTER(hVA_pc),
                               (gpointer) data);
        }
        g_mutex_unlock(&hashtable_lock);

        qemu_plugin_register_vcpu_mem_cb(insn, vcpu_mem_access,
                                         QEMU_PLUGIN_CB_NO_REGS,
                                         rw, data);

        qemu_plugin_register_vcpu_insn_exec_cb(insn, vcpu_insn_exec,
                                               QEMU_PLUGIN_CB_NO_REGS, data);
    }
}

static void insn_free(gpointer data)
{
    InsnData *insn = (InsnData *) data;
    g_free(insn);
}

static void plugin_exit(qemu_plugin_id_t id, void *p)
{
    g_hash_table_destroy(miss_ht);
}

typedef enum {
    OP_X,
    OP_1,
    OP_2,
    OP_3
} op_types_t;

QEMU_PLUGIN_EXPORT
int qemu_plugin_install(qemu_plugin_id_t id, const qemu_info_t *info,
                        int argc, char **argv)
{
    op_types_t op = OP_X;
    uint64_t value = 0;
    bool flag_set = false;

    for (int i = 0; i < argc; i++) {
        char *opt = argv[i];
        g_autofree char **tokens = g_strsplit(opt, "=", 2);

        if (g_strcmp0(tokens[0], "value") == 0) {
            value = STRTOLL(tokens[1]);
        } else if (g_strcmp0(tokens[0], "flag_set") == 0) {
            if (!qemu_plugin_bool_parse(tokens[0], tokens[1], &flag_set)) {
                fprintf(stderr, "boolean argument parsing failed: %s\n", opt);
                return -1;
            }
        } else if (g_strcmp0(tokens[0], "cpuFirst") == 0) {
            cpuFirst = STRTOLL(tokens[1]);
        } else if (g_strcmp0(tokens[0], "cpuLast") == 0) {
            cpuLast = STRTOLL(tokens[1]);
        } else if (g_strcmp0(tokens[0], "op_type") == 0) {
            if (g_strcmp0(tokens[1], "op1") == 0) {
                op = OP_1;
            } else if (g_strcmp0(tokens[1], "op2") == 0) {
                op = OP_2;
            } else if (g_strcmp0(tokens[1], "op3") == 0) {
                op = OP_3;
            } else {
                fprintf(stderr, "invalid op: %s\n", opt);
                return -1;
            }
        } else {
            fprintf(stderr, "option parsing failed: %s\n", opt);
            return -1;
        }
    }
    for (int cpu = 0; cpu < MAX_CPUS; cpu++) {
        last_insn_fetch[cpu].s.pc = 0;
    }

    qemu_plugin_qflex_get_callbacks(&qflex_callbacks);
    // Insert callback on translation block generation
    qemu_plugin_register_vcpu_tb_trans_cb(id, vcpu_tb_trans);
    // Insert callback on program exit
    qemu_plugin_register_atexit_cb(id, plugin_exit, NULL);

    g_autoptr(GString) params_log = g_string_new("");
    g_string_append_printf(params_log, "option parsing success: value[0x%016lx]:flag_set[%s]:op_type[%i]\n", value, flag_set ? "true" : "false", op);
    qemu_plugin_outs(params_log->str);


    miss_ht = g_hash_table_new_full(NULL, g_direct_equal, NULL, insn_free);

    return 0;
}
