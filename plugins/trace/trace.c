/*
 * [ Who ]
 *
 * [ What ]
 *
 * License: GNU GPL, version 2 or later.
 *   See the COPYING file in the top-level directory.
 */

#include <stdint.h>
#include <stdio.h>

#include <qemu-plugin.h>
#include "trace.h"


// Ensure that the plugin run only against the version
// it was compiled for
QEMU_PLUGIN_EXPORT int qemu_plugin_version = QEMU_PLUGIN_VERSION;

static GMutex lock;
static GHashTable* transaction_ptr;

/**
 * Free a translation cache entry from the GHashMap
 * This is mainly called on plugin destruction
 */
static void
trans_free(gpointer data)
{
    transaction_state_t* trans = (transaction_state_t *) data;
    g_string_free(trans->disas_str, true);
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


    transaction_state_t* state = (transaction_state_t*) userdata;

    struct qemu_plugin_hwaddr* hwaddr = qemu_plugin_get_hwaddr(info, vaddr);

    struct memory_transaction_state __attribute__((unused)) mem = {0};

    //! missing PC physical address
    mem.transaction.target_va_pc = state->target_va_pc;
    mem.transaction.instruction_bytes_size = state->instruction_bytes_size;
    mem.transaction.opcode = state->opcode;
    //? mem.transaction.type = SMTH


    mem.va_page = vaddr;
    mem.pa_page = qemu_plugin_hwaddr_phys_addr(hwaddr);
    mem.is_store = qemu_plugin_mem_is_store(info);
    mem.is_io = (hwaddr && qemu_plugin_hwaddr_is_io(hwaddr)); //? Works only for full system emulation

    //TODO call libqflex or FLEXUS

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

    transaction_state_t* state = (transaction_state_t*) userdata;
    struct instr_transaction_state __attribute__((unused)) instr = {0};

    instr.transaction.target_va_pc = state->target_va_pc;
    instr.transaction.instruction_bytes_size = state->instruction_bytes_size;
    instr.transaction.opcode = state->opcode;

    //TODO, maybe ?? instr.branch_type =

    //TODO call libqflex or FLEXUS
}

/**
 * Get called on every instruction translation
 */
static void
dispatch_vcpu_tb_trans(qemu_plugin_id_t id, struct qemu_plugin_tb* tb)
{
    //? Still not sure what to do about it
    // uint64_t block_start = qemu_plugin_tb_vaddr(tb);

    transaction_state_t* transaction;
    size_t nb_instruction = qemu_plugin_tb_n_insns(tb);

    // For each instruction in the translation block (TB)
    for (size_t i = 0; i < nb_instruction; i++)
    {
        struct qemu_plugin_insn* instruction = qemu_plugin_tb_get_insn(tb, i);

        size_t host_pa_pc = (size_t) qemu_plugin_insn_haddr(instruction);

        g_mutex_lock(&lock);
        transaction = g_hash_table_lookup(transaction_ptr, GSIZE_TO_POINTER(host_pa_pc));
        g_mutex_unlock(&lock);

        if (transaction == NULL)
        {
            transaction                         = g_new0(transaction_state_t, 1);
            transaction->target_va_pc           = qemu_plugin_insn_vaddr(instruction);
            transaction->host_pa_pc             = host_pa_pc;
            transaction->opcode                 = *(uint32_t*) qemu_plugin_insn_haddr(instruction); //? From the official plugins
            transaction->instruction_bytes_size = qemu_plugin_insn_size(instruction);
            transaction->disas_str              = g_string_new(qemu_plugin_insn_disas(instruction));

            g_mutex_lock(&lock);
            g_hash_table_insert(transaction_ptr, GSIZE_TO_POINTER(host_pa_pc), transaction);
            g_mutex_unlock(&lock);
        }

        qemu_plugin_register_vcpu_mem_cb(
            instruction,
            dispatch_memory_access,
            QEMU_PLUGIN_CB_NO_REGS,
            QEMU_PLUGIN_MEM_RW,
            (void*)transaction);

        qemu_plugin_register_vcpu_insn_exec_cb(
            instruction,
            dispatch_instruction,
            QEMU_PLUGIN_CB_NO_REGS ,
            (void*)transaction);
    }

}


static void
exit_plugin(qemu_plugin_id_t id, void* p)
{
    // ─── Logging Hashmap Translation Cache Size ──────────────────────────

    guint hashmap_size = g_hash_table_size(transaction_ptr);
    gfloat hashmap_M_space = hashmap_size * sizeof(transaction_state_t) / 1e6;

    char* size_logger   = g_strdup_printf("> HASH_MAP_SIZE: %i\n", hashmap_size);
    char* space_logger  = g_strdup_printf("> HASH_MAP_MBYTES: %f\n", hashmap_M_space);
    char* struct_size   = g_strdup_printf("> TRANSLATION_BYTES: %li\n", sizeof(transaction_state_t));

    qemu_plugin_outs(struct_size);
    qemu_plugin_outs(size_logger);
    qemu_plugin_outs(space_logger);

    g_free(struct_size);
    g_free(size_logger);
    g_free(space_logger);

    g_hash_table_destroy(transaction_ptr);
    qemu_plugin_outs("==> TRACE END");
}


/**
 * Plugin entry. Parse the arguments. Register the call back for each transaction,
 * as well as plugin exit.
 */
QEMU_PLUGIN_EXPORT int
qemu_plugin_install(qemu_plugin_id_t id , const qemu_info_t* info, int argc, char** argv)
{

    transaction_ptr = g_hash_table_new_full(
        NULL,
        g_direct_equal,
        NULL,
        trans_free);

    // Register translation callback
    qemu_plugin_register_vcpu_tb_trans_cb(id, dispatch_vcpu_tb_trans);
    // Register plugin's exit mechanism
    qemu_plugin_register_atexit_cb(id, exit_plugin, NULL);

    return 0;
}