#include "qemu/osdep.h"

#include "block/block-io.h"
#include "block/block_int-io.h"
#include "block/snapshot.h"
#include "io/channel-buffer.h"
#include "io/channel-file.h"
#include "io/channel-command.h"
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
#include "sysemu/replay.h"
#include "sysemu/runstate.h"

#include "snapvm-external.h"

static char *get_zstd(Error **errp) {
  char *zstd = g_find_program_in_path("zstd");
  if (!zstd)
    error_setg(errp, "zstd not found in PATH");

  return zstd;
}

bool load_snapshot_external(const char *name, const char *vmstate,
                            bool has_devices, strList *devices, Error **errp) {
  BlockDriverState *bs_vm_state;
  QEMUSnapshotInfo sn;
  QEMUFile *f;
  int ret;
  AioContext *aio_context;
  MigrationIncomingState *mis = migration_incoming_get_current();

  if (!bdrv_all_can_snapshot(has_devices, devices, errp)) {
    return false;
  }
  ret = bdrv_all_has_snapshot(name, has_devices, devices, errp);
  if (ret < 0) {
    return false;
  }
  if (ret == 0) {
    error_setg(errp, "Snapshot '%s' does not exist in one or more devices",
               name);
    return false;
  }

  bs_vm_state = bdrv_all_find_vmstate_bs(vmstate, has_devices, devices, errp);
  if (!bs_vm_state) {
    return false;
  }
  aio_context = bdrv_get_aio_context(bs_vm_state);

  /* Don't even try to load empty VM states */
  aio_context_acquire(aio_context);
  ret = bdrv_snapshot_find(bs_vm_state, &sn, name);
  aio_context_release(aio_context);

  char snapshot_name[293];
  snprintf(snapshot_name, sizeof(snapshot_name), "%s.zstd", sn.name);

  if (ret < 0) {
    return false;
  } else if (sn.vm_state_size == 0 &&
             !g_file_test(snapshot_name, G_FILE_TEST_IS_REGULAR)) {
    error_setg(errp, "This is a disk-only snapshot. Revert to it "
                     " offline using qemu-img");
    return false;
  }

  /*
   * Flush the record/replay queue. Now the VM state is going
   * to change. Therefore we don't need to preserve its consistency
   */
  replay_flush_events();

  /* Flush all IO requests so they don't interfere with the new state.  */
  bdrv_drain_all_begin();

  ret = bdrv_all_goto_snapshot(name, has_devices, devices, errp);
  if (ret < 0) {
    goto err_drain;
  }

  /* restore the VM state */
  if (g_file_test(snapshot_name, G_FILE_TEST_IS_REGULAR)) {
    char *zstd = get_zstd(errp);
    if (!zstd)
      return false;

    const char *args[] = {zstd, "-f", "-q",          "-T0",
                          "-d", "-c", snapshot_name, NULL};

    QIOChannelCommand *ioc =
        qio_channel_command_new_spawn(args, O_RDONLY, errp);
    if (!ioc) {
      error_setg(errp, "Could not create pipe for zstd");
      return false;
    }

    qio_channel_set_name(QIO_CHANNEL(ioc), "load_snapshot");

    f = qemu_file_new_input(QIO_CHANNEL(ioc));
    if (!f) {
      error_setg(errp, "Could not open VM state file");
      return false;
    }

    g_free(zstd);

  } else {
    f = qemu_fopen_bdrv(bs_vm_state, 0);
    if (!f) {
      error_setg(errp, "Could not open VM state file");
      goto err_drain;
    }
  }

  qemu_system_reset(SHUTDOWN_CAUSE_SNAPSHOT_LOAD);
  mis->from_src_file = f;

  if (!yank_register_instance(MIGRATION_YANK_INSTANCE, errp)) {
    ret = -EINVAL;
    goto err_drain;
  }
  aio_context_acquire(aio_context);
  ret = qemu_loadvm_state(f);
  migration_incoming_state_destroy();
  aio_context_release(aio_context);

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
