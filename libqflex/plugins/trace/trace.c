/*
 * [ Who ]
 *
 * [ What ]
 *
 * License: GNU GPL, version 2 or later.
 *   See the COPYING file in the top-level directory.
 */

#include "qemu/osdep.h"

#include "qemu/plugin-memory.h"
#include "qemu/qemu-plugin.h"
#include "target/arm/cpu.h"

#include "middleware/libqflex/libqflex-legacy-api.h"
#include "trace.h"


// Ensure that the plugin run only against the version
// it was compiled for
QEMU_PLUGIN_EXPORT int qemu_plugin_version = QEMU_PLUGIN_VERSION;

static GMutex lock;
static GHashTable* tb_table;

/**
 * Free a translation cache entry from the GHashMap
 * This is mainly called on plugin destruction
 */
static void
trans_free(gpointer data)
{
    trace_insn_t* trans = (trace_insn_t *) data;
    // g_string_free(trans->disas_str, true);
    g_free(trans);
}

/**
 * @brief Dispatches memory access.
 * @details Called on every translation of memory's accessing instruction.
 *          This will add the generic translation information to a new structure
 *
 * @param vcpu_index Index of the virtual CPU.
 * @param info Memory information.
 * @param vaddr OPage virtual address.
 * @param userdata Generic translation info.
 */
static void
dispatch_memory_access(unsigned int vcpu_index, qemu_plugin_meminfo_t info, uint64_t vaddr, void* userdata)
{

    trace_insn_t* insn = (trace_insn_t*) userdata;

    /**
     * Checking if the decoder and QEMU agree on the type of memory access
     * Usless possibly, only useful to retrieve info about atomic store or load
     */
    struct mem_access mem_info = {0};
    g_assert(decode_armv8_mem_opcode(&mem_info, insn->opcode));
    g_assert(mem_info.is_store == qemu_plugin_mem_is_store(info));

    // ─────────────────────────────────────────────────────────────────────


    struct qemu_plugin_hwaddr* hwaddr = qemu_plugin_get_hwaddr(info, vaddr);


    memory_transaction_t tr = {0};

    tr.io = (hwaddr && qemu_plugin_hwaddr_is_io(hwaddr));

    tr.s.pc              = insn->target_pc_va;
    tr.s.opcode          = insn->opcode;
    tr.s.logical_address = vaddr;
    tr.s.exception       = insn->exception_lvl;
    //? function used previously to get the phys_addr
    //? arm_cpu_get_phys_page_attrs_debug(CPUState *cs, vaddr addr, MemTxAttrs *attrs)
    tr.s.physical_address = hwaddr->phys_addr;  //! What the heck do we need this

    tr.s.size   = mem_info.size;
    tr.s.atomic = mem_info.is_atomic;
    tr.s.type   = mem_info.is_store ? QEMU_Trans_Store : QEMU_Trans_Load;


    flexus_api.trace_mem(vcpu_index, &tr);
}

/**
 * @brief Dispatches instruction.
 * @details Called on every translation of instruction.
 *          This will add the generic translation information to a new structure
 *          specifically for instruction that do NOT access memory
 *
 * @param vcpu_index Index of the virtual CPU.
 * @param userdata Generic translation info.
 */
static void
dispatch_instruction(unsigned int vcpu_index, void* userdata)
{

    trace_insn_t* insn = (trace_insn_t*) userdata;
    g_assert(insn->target_pc_va);

    branch_type_t br_type;
    memory_transaction_t tr = {0};

    tr.io = false;

    tr.s.pc               = insn->target_pc_va;
    tr.s.opcode           = insn->opcode;
    tr.s.exception        = insn->exception_lvl;
    tr.s.logical_address  = insn->target_pc_va;
    tr.s.physical_address = insn->host_pc_pa;

    tr.s.size        = insn->byte_size;
    tr.s.branch_type = decode_armv8_branch_opcode(&br_type, insn->opcode) ? br_type : QEMU_Non_Branch;
    tr.s.type        = QEMU_Trans_Instr_Fetch;

    flexus_api.trace_mem(vcpu_index, &tr);
}

/**
 * Get called on every instruction translation
 */
static void
dispatch_vcpu_tb_trans(qemu_plugin_id_t id, struct qemu_plugin_tb* tb)
{
    //? Still not sure what to do about it
    // uint64_t block_start = qemu_plugin_tb_vaddr(tb);

    trace_insn_t* transaction = NULL;
    size_t nb_instruction = qemu_plugin_tb_n_insns(tb);

    // For each instruction in the translation block (TB)
    for (size_t i = 0; i < nb_instruction; i++)
    {
        struct qemu_plugin_insn* insn = qemu_plugin_tb_get_insn(tb, i);

        physical_address_t host_pc_pa = (u_int64_t) qemu_plugin_insn_haddr(insn);

        g_mutex_lock(&lock);
        transaction = g_hash_table_lookup(tb_table, GSIZE_TO_POINTER(host_pc_pa));
        g_mutex_unlock(&lock);

        if (transaction == NULL)
        {
            transaction                         = g_new0(trace_insn_t, 1);

            transaction->host_pc_pa             = host_pc_pa;
            transaction->target_pc_va           = qemu_plugin_insn_vaddr(insn);
            transaction->opcode                 = * (uint32_t*)qemu_plugin_insn_haddr(insn); //? From the official plugins
            transaction->byte_size              = qemu_plugin_insn_size(insn);
            transaction->disas_str              = qemu_plugin_insn_disas(insn);
            transaction->exception_lvl          = arm_current_el(&ARM_CPU(current_cpu)->env);

            g_mutex_lock(&lock);
            g_hash_table_insert(tb_table, GSIZE_TO_POINTER(host_pc_pa), transaction);
            g_mutex_unlock(&lock);
        }

        qemu_plugin_register_vcpu_mem_cb(
            insn,
            dispatch_memory_access,
            QEMU_PLUGIN_CB_NO_REGS,
            QEMU_PLUGIN_MEM_RW,
            (void*)transaction);

        qemu_plugin_register_vcpu_insn_exec_cb(
            insn,
            dispatch_instruction,
            QEMU_PLUGIN_CB_NO_REGS ,
            (void*)transaction);
    }
}


static void
exit_plugin(qemu_plugin_id_t id, void* p)
{
    // ─── Logging Hashmap Translation Cache Size ──────────────────────────

    guint hashmap_size = g_hash_table_size(tb_table);
    gfloat hashmap_M_space = hashmap_size * sizeof(trace_insn_t) / 1e6;

    char* size_logger   = g_strdup_printf("> HASH_MAP_SIZE: %i\n", hashmap_size);
    char* space_logger  = g_strdup_printf("> HASH_MAP_MBYTES: %f\n", hashmap_M_space);
    char* struct_size   = g_strdup_printf("> TRANSLATION_BYTES: %li\n", sizeof(trace_insn_t));

    qemu_plugin_outs(struct_size);
    qemu_plugin_outs(size_logger);
    qemu_plugin_outs(space_logger);

    g_free(struct_size);
    g_free(size_logger);
    g_free(space_logger);

    g_hash_table_destroy(tb_table);
    qemu_plugin_outs("==> TRACE END");
}


/**
 * Plugin entry. Parse the arguments. Register the call back for each transaction,
 * as well as plugin exit.
 */
void
libqflex_trace_init(void)
{

    qemu_plugin_id_t qflex_trace_id = qemu_plugin_register_builtin();

    tb_table = g_hash_table_new_full(
        NULL,
        g_direct_equal,
        NULL,
        trans_free);

    // Register translation callback
    qemu_plugin_register_vcpu_tb_trans_cb(qflex_trace_id, dispatch_vcpu_tb_trans);
    // Register plugin's exit mechanism
    qemu_plugin_register_atexit_cb(qflex_trace_id, exit_plugin, NULL);
}