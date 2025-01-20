#include "qemu/osdep.h"

#include "block/block-io.h"
#include "block/block_int-io.h"
#include "block/snapshot.h"
#include "io/channel-buffer.h"
#include "io/channel-file.h"
#include "migration/global_state.h"
#include "migration/migration.h"
#include "migration/qemu-file.h"
#include "migration/savevm.h"
#include "migration/options.h"
#include "monitor/hmp.h"
#include "monitor/monitor.h"
#include "qapi/error.h"
#include "qapi/qapi-builtin-visit.h"
#include "qapi/qapi-commands-block.h"
#include "qapi/qmp/qdict.h"
#include "qapi/qmp/qerror.h"
#include "qapi/util.h"
#include "qemu/cutils.h"
#include "qemu/log.h"
#include "qemu/main-loop.h"
#include "qemu/typedefs.h"
#include "sysemu/replay.h"
#include "sysemu/runstate.h"

#include "snapvm-external.h"

#include <gio/gio.h> /* Required for GIO error codes */
#include <zstd.h>


bool save_snapshot_external(const char *name, bool overwrite,
                            const char *vmstate, bool has_devices,
                            strList *devices, Error **errp)
{
        QEMUSnapshotInfo sn1, *sn = &sn1;
        QEMUFile *f;
        int ret = -1, ret2;
        int saved_vm_running;

        GLOBAL_STATE_CODE();

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

        /* Delete old snapshots of the same name */
        if (name && overwrite) {
                if (bdrv_all_delete_snapshot(name, has_devices, devices, errp) <
                    0) {
                        return false;
                }
        }

        saved_vm_running = runstate_is_running();
        global_state_store();
        vm_stop(RUN_STATE_SAVE_VM);
        bdrv_drain_all_begin();

        memset(sn, 0, sizeof(*sn));
        /* fill auxiliary fields */
        g_autoptr(GDateTime) now = g_date_time_new_now_local();
        sn->date_sec = g_date_time_to_unix(now);
        sn->date_nsec = g_date_time_get_microsecond(now) * 1000;
        sn->vm_clock_nsec = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL);
        if (replay_mode != REPLAY_MODE_NONE) {
                sn->icount = replay_get_current_icount();
        } else {
                sn->icount = -1ULL;
        }

        if (name) {
                pstrcpy(sn->name, sizeof(sn->name), name);
        } else {
                g_autofree char *autoname =
                    g_date_time_format(now, "vm-%Y_%m_%d-%H%M_%S");
                pstrcpy(sn->name, sizeof(sn->name), autoname);
        }

        /* save the VM state to the specified output file */
        g_autofree char *output_state_file =
            g_build_filename(qemu_snapvm_state.path, sn->name, NULL);

        QIOChannelFile *ioc;
        ioc = qio_channel_file_new_path(
            output_state_file, O_WRONLY | O_CREAT | O_TRUNC, 0660, errp);
        f = qemu_file_new_output(QIO_CHANNEL(ioc));

        if (!f) {
                error_setg(errp, "Could not open output file %s",
                           output_state_file);
                goto the_end;
        }

        ret = qemu_savevm_state(f, errp);
        ret2 = qemu_fclose(f);

        /**
         * TODO: Create new thread that will compress the outputed file using
         * libzstd
         * */

        if (ret < 0) {
                goto the_end;
        }
        if (ret2 < 0) {
                ret = ret2;
                goto the_end;
        }

        ret = 0;

the_end:
        bdrv_drain_all_end();
        if (saved_vm_running) {
                vm_start();
        }
        return ret == 0;
}
