#include "qemu/osdep.h"

#include "savevm-external.h"

bool save_snapshot_external(void)
{

    // Make sure that we are in the main thread, and not
    // concurrently accessing the ressources
    GLOBAL_STATE_CODE();

    /**
     * Internal snapshot do not allow this, therefor we repliacte this behaviour
     */
    if (migration_is_blocked(errp)) {
        return false;
    }

    if (!replay_can_snapshot()) {
        error_setg(errp, "Record/replay does not allow making snapshot "
                   "right now. Try once more later.");
        return false;
    }

    if (!bdrv_all_can_snapshot(has_devices, devices, errp)) {
        return false;
    }

    // TODO: Some kind of logic about the name

    BlockDriverState* bs = bdrv_all_find_vmstate_bs(
                                                vmstate,
                                                has_devices,
                                                devices,
                                                errp);
    if (bs == NULL) {
        return false;
    }

    global_state_store();
    vm_stop(RUN_STATE_SAVE_VM);


    //
    // 3. Get all block device, that could be snapshoted
        // 3.1 Save block for all block
    // 4. Save mem
    // 5. restart if needed

}