#include <stdint.h>
#include <qemu-plugin.h>


// Ensure that the plugin run only against the version
// it was compiled for
QEMU_PLUGIN_EXPORT int qemu_plugin_version = QEMU_PLUGIN_VERSION;


static qemu_plugin_vcpu_mem_cb_t
dispatch_memory_access(unsigned int vcpu_index, qemu_plugin_meminfo_t info, uint64_t vaddr, void* userdata)
{}

static qemu_plugin_vcpu_udata_cb_t
dispatch_instruction(unsigned int vcpu_index, void* userdata)
{}

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

        qemu_plugin_register_vcpu_mem_cb(
            instruction, 
            dispatch_memory_access, 
            QEMU_PLUGIN_CB_NO_REGS,
            QEMU_PLUGIN_MEM_RW, 
            NULL);

        qemu_plugin_register_vcpu_insn_exec_cb(
            instruction,
            dispatch_instruction, 
            QEMU_PLUGIN_CB_NO_REGS ,
            NULL);
    } 
}


static void
exit_plugin(qemu_plugin_id_t id, void* p)
{
    qemu_plugin_outs("==> TRACE END");
}

// Step 1 get a list of the event
// PC
// MEM access (physical)
// list of event for specific instruction executed
// function to access main memory

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