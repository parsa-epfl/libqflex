#include "qemu/osdep.h"

#include "block/block-io.h"
#include "block/block_int-io.h"
#include "block/snapshot.h"
#include "io/channel-buffer.h"
#include "io/channel-command.h"
#include "io/channel-file.h"
#include "migration/migration.h"
#include "migration/qemu-file.h"
#include "migration/savevm.h"
#include "monitor/hmp.h"
#include "monitor/monitor.h"
#include "qapi/error.h"
#include "qapi/qmp/qdict.h"
#include "qemu/error-report.h"
#include "qemu/log.h"
#include "qemu/yank.h"
#include "snapvm-external.h"
#include "sysemu/replay.h"
#include "sysemu/runstate.h"

bool load_snapshot_external(const char *name, const char *vmstate,
                            bool has_devices, strList *devices, Error **errp)
{
        QEMUFile *f;
        int ret;
        MigrationIncomingState *mis = migration_incoming_get_current();

        if (!bdrv_all_can_snapshot(has_devices, devices, errp)) {
                return false;
        }

        /*
         * Flush the record/replay queue. Now the VM state is going
         * to change. Therefore we don't need to preserve its consistency
         */
        replay_flush_events();

        /* Flush all IO requests so they don't interfere with the new state.  */
        bdrv_drain_all_begin();

        g_autofree char* snapshot_name = g_build_filename(qemu_snapvm_state.path, name, NULL);
        if (!g_file_test(snapshot_name, G_FILE_TEST_IS_REGULAR))
        {
                error_setg(errp, "Could not open .zts file: %s", snapshot_name);
        }

        g_autofree char *zstd = g_find_program_in_path("zstd");

        if (!zstd) 
        {
            error_setg(errp, "zstd not found in PATH");
            return false;
        }

        const char *args[] = {zstd, "-f", "-q", "-T0", "-d", "-c", snapshot_name, NULL};

        QIOChannelCommand *ioc = qio_channel_command_new_spawn(args, O_RDONLY, errp);

        if (!ioc) {
                error_setg(errp, "Could not create pipe for zstd");
                return false;
        }

        qio_channel_set_name(QIO_CHANNEL(ioc), "load_snapshot_external");

        f = qemu_file_new_input(QIO_CHANNEL(ioc));

        if (!f) {
                error_setg(errp, "Could not open VM state file");
                return false;
        }

        qemu_system_reset(SHUTDOWN_CAUSE_SNAPSHOT_LOAD);
        mis->from_src_file = f;

        if (!yank_register_instance(MIGRATION_YANK_INSTANCE, errp)) {
                ret = -EINVAL;
                goto err_drain;
        }

        //aio_context_acquire(aio_context);
        ret = qemu_loadvm_state(f);
        migration_incoming_state_destroy();
        //aio_context_release(aio_context);

        bdrv_drain_all_end();

        if (ret < 0) {
                error_setg(errp, "Error %d while loading VM state", ret);
                return false;
        }

        return true;

err_drain:
        bdrv_drain_all_end();
        return false;
}
