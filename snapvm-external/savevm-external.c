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

#include <gio/gio.h>

#include "snapvm-external.h"

/**
 * There is no name, therefor it's an increment
 * of the previous disk image.
 *
 * If it has been loaded before, put it asside of the root,
 * otherwise put it in a tmp folder
 */
// static void
// save_bdrv_new_increment(
//     SnapTransaction* trans,
//     Error** errp
//)
//{}

/**
 * Create a new root external snapshot.
 * A name is passed by and a copy of the disk
 * should be made to avoid altering the original disk
 */
// static void
// save_bdrv_new_root(
//     SnapTransaction* trans,
//     Error** errp
//     )
//{
//
//     g_mkdir_with_parents(trans->new_bdrv.dirname , 0700);
//
//     if (link(trans->root_bdrv.fullpath, trans->new_bdrv.fullpath) < 0)
//     {
//         error_setg(
//             errp,
//             "Could not move external snapshot bdrv (block device) %s to %s",
//             trans->root_bdrv.fullpath,
//             trans->new_bdrv.fullpath);
//     }
//
//
//     trans->new_bdrv.bs = trans->root_bdrv.bs;
//     bdrv_update_filename(trans->new_bdrv.bs, trans->new_bdrv.fullpath);
//
//     // Consider that know we have loaded a snapshot and all next
//     // logic should behave accordingly.
//     qemu_snapvm_ext_state.has_been_loaded = true;
// }

/**
 * Save device/block drive state into a full snapshot
 * or an incremental snapshot.
 *
 * Case 1: A name is passed by and a copy of the disk should be
 * made.
 *
 * Case 2: There is no name, therefor it's an increment
 * of the previous disk image.
 *      Sub Case 1: The root image is still the very base image
 *          -> add increment in 'tmp' dir
 *      Sub Case 2: The root image is a previously full snapshot save (Case 1)
 *                  and the check point have to be incremented in the dir
 * TODO: handle relative backing file path
 */
// static bool
// save_snapshot_external_bdrv(
//     SnapTransaction* trans,
//     Error** errp)
//{
//
//     switch (trans->mode)
//     {
//     case INCREMENT:
//         save_bdrv_new_increment(trans, errp);
//         break;
//
//     case NEW_ROOT:
//         g_assert(trans->new_name != NULL);
//         save_bdrv_new_root(trans, errp);
//         break;
//
//     default:
//         g_assert_not_reached();
//     }
//
//
//     if (*errp)
//     {
//         error_report_err(*errp);
//         return false;
//     }
//
//     return true;
//
// }

// static bool
// save_snapshot_external_mem(
//     SnapTransaction const * trans,
//     Error** errp)
//{
//
//     g_autoptr(GString) new_mem_filename = g_string_new("mem");
//
//     switch(trans->mode)
//     {
//         case NEW_ROOT:
//         break;
//
//         case INCREMENT:
//         g_string_append_printf(new_mem_filename, "-%s", trans->datetime);
//         break;
//
//         default:
//         g_assert_not_reached();
//     }
//
//     g_autofree char* new_mem_path = g_build_path(G_DIR_SEPARATOR_S,
//     trans->new_bdrv.dirname, new_mem_filename->str, NULL);
//
//     QIOChannelFile* ioc = qio_channel_file_new_path(
//         new_mem_path,
//         O_WRONLY | O_CREAT | O_TRUNC,
//         0660,
//         errp);
//
//     qio_channel_set_name(QIO_CHANNEL(ioc),
//     "snapvm-external-memory-outgoing");
//
//     if (!ioc)
//     {
//         error_setg(errp, "Could not open channel");
//         goto end;
//     }
//
//     QEMUFile* f = qemu_file_new_output(QIO_CHANNEL(ioc));
//
//     if (!f)
//     {
//         error_setg(errp, "Could not open VM state file");
//         goto end;
//     }
//
//     object_unref(OBJECT(ioc));
//
//     int16_t ret = qemu_savevm_state(f, errp);
//     // Perfrom noflush and return total number of bytes transferred
//     qemu_file_transferred(f);
//
//     if (ret < 0 || qemu_fclose(f) < 0)
//         error_setg(errp, QERR_IO_ERROR);
//
// end:
//
//     if (*errp)
//     {
//         error_report_err(*errp);
//         return false;
//     }
//
//     return true;
// }

// }

bool save_snapshot_external(const char *name, bool overwrite,
                            const char *vmstate, bool has_devices,
                            strList *devices, Error **errp)
{
        QEMUSnapshotInfo sn1, *sn = &sn1;
        int ret = -1, ret2;
        QEMUFile *f;
        int saved_vm_running;
        // uint64_t vm_state_size;
        g_autoptr(GDateTime) now = g_date_time_new_now_local();
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
                    g_date_time_format(now, "vm-%Y%m%d%H%M%S");
                pstrcpy(sn->name, sizeof(sn->name), autoname);
        }

        /* save the VM state to the specified output file */
        g_autofree char *output_state_file =
            g_build_filename(qemu_snapvm_state.path, name, NULL);
        // f = g_fopen(output_state_file, "wb");

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
        // vm_state_size = qemu_file_transferred(f);
        ret2 = qemu_fclose(f);
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

// bool save_snapshot_external(SnapTransaction *trans, Error **errp)
//{
//
//     bool ret = true;
//     g_autoptr(GList) bdrvs = NULL;
//     AioContext* aio_context = NULL;
//     int saved_vm_running = runstate_is_running();
//
//     // Make sure that we are in the main thread, and not
//     // concurrently accessing the ressources
//     GLOBAL_STATE_CODE();
//
//     // ─── Vm State Checkup
//     ────────────────────────────────────────────────
//
//     /**
//      * Internal snapshot do not allow this, therefor we repliacte this
//      behaviour
//      */
//     if (migration_is_blocked(errp))
//     {
//         ret =  false;
//         goto end;
//     }
//
//     if (!replay_can_snapshot())
//     {
//         error_setg(errp, "Record/replay does not allow making snapshot
//         "
//                    "right now. Try once more later.");
//
//         ret = false;
//         goto end;
//     }
//
//     //? Test if all drive are snapshot-able
//     if (!bdrv_all_can_snapshot(false, NULL, errp)) { return false; }
//
//     trans->root_bdrv.bs = bdrv_all_find_vmstate_bs(NULL, false, NULL,
//     errp); if (trans->root_bdrv.bs == NULL)
//     {
//         ret =  false;
//         goto end;
//     }
//
//     // Retrieve all snaspshot able drive
//     if (bdrv_all_get_snapshot_devices( false, NULL, &bdrvs, errp))
//     {
//         ret =  false;
//         goto end;
//     }
//
//     // No stupid behaviour
//     g_assert(bdrvs != NULL);
//
//         // Trigger the global state to be saved within the snapshot
//     global_state_store();
//     vm_stop(RUN_STATE_SAVE_VM);
//
//     // Acquire lock before doing anything
//     aio_context = bdrv_get_aio_context(trans->root_bdrv.bs);
//     aio_context_acquire(aio_context);
//
//     // ─── Generate Path
//     ───────────────────────────────────────────────────
//
//     /* If needed */
//     trans->root_bdrv.fullpath   =
//     get_base_bdrv(trans->root_bdrv.bs)->filename;
//     trans->root_bdrv.dirname    =
//     g_path_get_dirname(trans->root_bdrv.fullpath);
//     trans->root_bdrv.basename   =
//     g_path_get_basename(trans->root_bdrv.fullpath); trans->datetime =
//     get_datetime();
//
//     switch (trans->mode)
//     {
//     case INCREMENT:
//     {
//
//         GString* new_filename_buf = g_string_new("");
//         /**
//          * Take the filename of the root image, and append the
//          datetime to it
//          */
//
//         g_string_append_printf(new_filename_buf, "%s-%s",
//         trans->root_bdrv.basename, trans->datetime);
//         trans->new_bdrv.basename = g_string_free(new_filename_buf,
//         false);
//
//         char const * const tmp_dir =
//         qemu_snapvm_ext_state.has_been_loaded ? NULL : "tmp";
//         trans->new_bdrv.fullpath = g_build_path(
//                                         G_DIR_SEPARATOR_S,
//                                         trans->root_bdrv.dirname,
//                                         tmp_dir, // is skipped if null
//                                         trans->new_bdrv.basename,
//                                         NULL);
//
//         trans->new_bdrv.dirname    =
//         g_path_get_dirname(trans->new_bdrv.fullpath); break;
//     }
//     case NEW_ROOT:
//     {
//
//         trans->new_bdrv.basename = trans->root_bdrv.basename;
//         trans->new_bdrv.dirname  = g_build_path(G_DIR_SEPARATOR_S,
//         trans->root_bdrv.dirname, trans->new_name, NULL);
//         trans->new_bdrv.fullpath = g_build_path(G_DIR_SEPARATOR_S,
//         trans->new_bdrv.dirname, trans->root_bdrv.basename, NULL);
//
//         break;
//
//     }
//     default:
//         g_assert_not_reached();
//     }
//
//     // Failsafe
//     g_assert(g_str_is_ascii(trans->datetime));
//     g_assert(g_str_is_ascii(trans->root_bdrv.basename));
//     g_assert(g_str_is_ascii(trans->root_bdrv.dirname));
//     g_assert(g_str_is_ascii(trans->root_bdrv.fullpath));
//     g_assert(g_str_is_ascii(trans->new_bdrv.basename));
//     g_assert(g_str_is_ascii(trans->new_bdrv.dirname));
//     g_assert(g_str_is_ascii(trans->new_bdrv.fullpath));
//
//
//     // ─── Iterate And Save BDRV
//     ───────────────────────────────────────────
//
//     GList* iterbdrvs = bdrvs;
//
//     while (iterbdrvs) {
//         trans->root_bdrv.bs = iterbdrvs->data;
//         if (!bdrv_is_read_only(trans->root_bdrv.bs)) {
//             ret = save_snapshot_external_bdrv(trans, errp);
//
//             if (!ret) goto end;
//             trans->new_bdrv.fullpath = trans->root_bdrv.bs->filename;
//         }
//         iterbdrvs = iterbdrvs->next;
//     }
//
//     // ─── Saving Main Memory
//     ──────────────────────────────────────────────
//
//     ret = save_snapshot_external_mem(trans, errp);
//
////     if (!ret) goto end;
//
//// #ifdef CONFIG_LIBQFLEX
////     if (qemu_libqflex_state.is_running) {
////         g_autoptr(GString) dst_path = g_string_new("");
//
////         g_autofree char* base_bdrv_dirname =
/// g_path_get_dirname(new_bdrv_path); /
/// g_string_append_printf(dst_path,
///"%s/%s", base_bdrv_dirname, snap_name);
//
////         if (checkpoint_num != -1)
////             g_string_append_printf(dst_path, "/checkpoint_%i",
/// checkpoint_num);
//
////         // call flexus API to trigger the cache memory dump
////         flexus_api.qmp(QMP_FLEXUS_DOSAVE, dst_path->str);
////     }
//// #endif
//
//    // ─── Return Gracefully Or... Not
//    ─────────────────────────────────────
//
//    // Release context lock
//    aio_context_release(aio_context);
//    aio_context = NULL;
//
// end:
//
//    if (aio_context)
//        aio_context_release(aio_context);
//
//    if (!ret)
//        error_report_err(*errp);
//
//    if (saved_vm_running)
//        vm_start();
//
//    return ret;
// return 0;
//}

// ─────────────────────────────────────────────────────────────────────────────
