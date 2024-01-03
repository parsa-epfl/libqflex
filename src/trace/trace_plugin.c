#include <qemu-plugin.h>


QEMU_PLUGIN_EXPORT int qemu_plugin_version = QEMU_PLUGIN_VERSION;


/**
 * get called on every instruction translation
 */
static void 
dispatch_vcpu_tb_trans(qemu_plugin_id_t id, struct qemu_plugin_tb* tb)
{
    // receive a translation block
    // for each instruction in the block do X

    // ..

    //
}


// Step 1 get a list of the event
// PC
// MEM access (physical)
// list of event for specific instruction  executed
// function to access main memory
