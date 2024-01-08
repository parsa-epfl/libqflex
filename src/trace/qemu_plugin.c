#include <glib.h>
#include <stdint.h>
#include <qemu-plugin.h>
#include "api.h"


// Ensure that the plugin run only against the version
// it was compiled for
QEMU_PLUGIN_EXPORT int qemu_plugin_version = QEMU_PLUGIN_VERSION;


static qemu_plugin_vcpu_mem_cb_t
dispatch_memory_access(unsigned int vcpu_index, qemu_plugin_meminfo_t info, uint64_t vaddr, transaction_state_t* userdata)
{



    struct qemu_plugin_hwaddr* hwaddr = qemu_plugin_get_hwaddr(info, vaddr);

    struct memory_transaction_state mem = {0};

    //! missing PC physical address
    mem.transaction.target_va_pc = userdata->target_va_pc;
    mem.transaction.instruction_bytes_size = userdata->instruction_bytes_size;
    mem.transaction.opcode = userdata->opcode;
    //? mem.transaction.type = SMTH
    
    
    mem.va_page = vaddr;
    mem.pa_page = qemu_plugin_hwaddr_phys_addr(hwaddr);
    mem.is_store = qemu_plugin_mem_is_store(info);
    mem.is_io = (hwaddr && qemu_plugin_hwaddr_is_io(hwaddr)); //? Works only for full system emulation


    g_free(userdata);
    qemu_plugin_outs("dispatch_memory_access")
    // Call QFLEX

}

static qemu_plugin_vcpu_udata_cb_t
dispatch_instruction(unsigned int vcpu_index, transaction_state_t* userdata)
{

    struct instr_transaction_state instr = {0};

    instr.transaction.target_va_pc = userdata->target_va_pc;
    instr.transaction.instruction_bytes_size = userdata->instruction_bytes_size;
    instr.transaction.opcode = userdata->opcode;

    //TODO, maybe ?? instr.branch_type = 


    g_free(userdata);
    qemu_plugin_outs("dispatch_instruction");

}

/**
 * Get called on every instruction translation
 */
static void 
dispatch_vcpu_tb_trans(qemu_plugin_id_t id, struct qemu_plugin_tb* tb)
{
    
    size_t nb_instruction = qemu_plugin_tb_n_insns(tb);

    for (size_t i = 0; i < nb_instruction; i++)
    {
        struct qemu_plugin_insn* instruction = qemu_plugin_tb_get_insn(tb, i);

        transaction_state_t* transaction = g_new0(transaction_state_t, 1); 
        transaction.target_va_pc    = qemu_plugin_insn_vaddr(instruction);
        transaction.opcode          = *(uint32_t*) qemu_plugin_insn_haddr(instruction); //? From the official plugins
        transaction.instruction_bytes_size = qemu_plugin_insn_size(instruction);

        qemu_plugin_register_vcpu_mem_cb(
            instruction, 
            dispatch_memory_access, 
            QEMU_PLUGIN_CB_NO_REGS,
            QEMU_PLUGIN_MEM_RW, 
            transaction);

        qemu_plugin_register_vcpu_insn_exec_cb(
            instruction,
            dispatch_instruction, 
            QEMU_PLUGIN_CB_NO_REGS ,
            transaction);
    } 
}


static void
exit_plugin(qemu_plugin_id_t id, void* p)
{
    qemu_plugin_outs("==> TRACE END");
}


/**
 * Plugin entry. Parse the arguments. Register the call back for each transaction, 
 * as well as plugin exit.
 */
QEMU_PLUGIN_EXPORT int 
qemu_plugin_install(qemu_plugin_id_t id , const qemu_info_t* info, int argc, char** argv)
{
    // Step 1: Parse arguments
    // Step 2: Register callback

    // Register translation callback
    qemu_plugin_register_vcpu_tb_trans_cb(id, dispatch_vcpu_tb_trans);

    // Regsiter plugin exit mechanism
    qemu_plugin_register_atexit_cb(id, exit_plugin, NULL);

    return 0;
}