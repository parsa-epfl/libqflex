#include "qemu/osdep.h"

#include "migration/migration.h"
#include "migration/global_state.h"
#include "block/block-io.h"
#include "block/block_int-io.h"
#include "block/snapshot.h"
#include "qapi/util.h"
#include "qapi/qapi-builtin-visit.h"
#include "qapi/error.h"
#include "qemu/cutils.h"
#include "qemu/main-loop.h"
#include "qemu/typedefs.h"
#include "sysemu/replay.h"
#include "sysemu/runstate.h"

#ifdef CONFIG_LIBQFLEX
#include "middleware/libqflex/legacy-qflex-api.h"
#include "middleware/libqflex/libqflex-module.h"
#endif

#include "savevm-external.h"


/**
 * Iterate over block driver's backing file chain until reaching
 * the firstmost block driver
 *
 * Iterrative version of the following code jff vvv
 *-- static BlockDriverState*
 *-- get_base_bdrv(BlockDriverState* bs_root) {
 *--     if (!bs_root || !bs_root->backing)
 *--         return bs_root;
 *--     return get_base_bdrv(child_bs(bs_root->backing));
 *-- }
 */

static BlockDriverState*
get_base_bdrv(BlockDriverState* bs_root) {
    BlockDriverState* bs = bs_root;
    while (bs && bs->backing) {
        bs = child_bs(bs->backing);
    }
    return bs;
}

static bool
get_snapshot_directory(
    GString* export_path,
    char const * const base_bdrv_filename,
    char const * const snap_name,
    int checkpoint_num)
{
    g_autofree char* dir_path   = g_path_get_dirname(base_bdrv_filename);
    g_autofree char* filename   = g_path_get_basename(base_bdrv_filename);

    g_assert(dir_path && filename);

    /**
     * Create a new const string to get the snap name if needed,
     * to avoid modifying trying to modify the NULL constant
     */
    char const * const folder_name = (snap_name == NULL) ? "tmp" : snap_name;
    const char* format = checkpoint_num >= 0 ? "%s/%s/checkpoint_%i/" : "%s/%s/";

    //! Need to check if this really work the way i think it does
    g_string_append_printf(export_path, format, dir_path, folder_name, checkpoint_num);
    // Always append the filename whatever append
    g_string_append_printf(export_path, "%s", filename);
    g_mkdir_with_parents(g_path_get_dirname(export_path->str), 0700);

    return true;
}

/**
 *? Make all block drive filepath *point* to the same location
 */
static void
bdrv_update_filename(BlockDriverState* bs, const char* path) {
    pstrcpy(bs->filename, sizeof(bs->filename), path);
    pstrcpy(bs->exact_filename, sizeof(bs->exact_filename), path);
    pstrcpy(bs->file->bs->filename, sizeof(bs->filename), path);
    pstrcpy(bs->file->bs->exact_filename, sizeof(bs->exact_filename), path);
    bdrv_refresh_filename(bs);
}

/**
 *
 */
static bool
save_snapshot_external_bdrv(
    BlockDriverState* bs,
    const char* snap_name,
    int checkpoint_num,
    Error** errp)
{
    // ─── Format Snapshot Name ────────────────────────────────────────────

    char tmp_filename[PATH_MAX] = {0};
    char const * const base_dir = get_base_bdrv(bs)->filename;

    g_autoptr(GString) snap_path_dst = g_string_new("");
    g_autoptr(GString) snap_path_tmp = g_string_new("");

    g_autoptr(GDateTime) now = g_date_time_new_now_local();
    g_autofree char* autoname = g_date_time_format(now, "%m-%d-%H%M_%S_%f");

    get_snapshot_directory(snap_path_tmp, base_dir, NULL, checkpoint_num);
    g_string_append_printf(snap_path_tmp, "-%s", autoname);

    if (snap_name)
    {
        pstrcpy(tmp_filename, sizeof(bs->filename), bs->filename);
        get_snapshot_directory(snap_path_dst, base_dir, snap_name, checkpoint_num);

        // Move temporary block device to snapshot save location
        if(link(tmp_filename, snap_path_dst->str) < 0 || unlink(tmp_filename) < 0)
        {
            error_setg(
                errp,
                "Could not move external snapshot bdrv (block device) %s to %s",
                tmp_filename,
                snap_path_dst->str);
        }

        bdrv_update_filename(bs, snap_path_dst->str);
    }


    // ─── Create Or Update Previous Snapshot ──────────────────────────────


    //TODO Create new tmp BDRV
    //TODO snapshot sync

    if (*errp)
    {
        error_report_err(*errp);
        return false;
    }

    return true;

}

static bool
save_snapshot_external_mem(
    const char *base_bdrv_filename,
    const char *snap_name,
    int checkpoint_num,
    Error **errp)
{
    return true;
}

bool save_snapshot_external(
    const char* snap_name,    // Snapshot output name
    const char* vm_state,
    bool has_devices,
    strList* devices,
    int checkpoint_num,
    Error** errp)
{

    bool ret = true;
    RunState saved_state = runstate_get();

    // Make sure that we are in the main thread, and not
    // concurrently accessing the ressources
    GLOBAL_STATE_CODE();

    // ─── Vm State Checkup ────────────────────────────────────────────────

    /**
     * Internal snapshot do not allow this, therefor we repliacte this behaviour
     */
    if (migration_is_blocked(errp))
    {
        ret =  false;
        goto end;
    }

    if (!replay_can_snapshot())
    {
        error_setg(errp, "Record/replay does not allow making snapshot "
                   "right now. Try once more later.");

        ret =  false;
        goto end;
    }

    //? Test if all drive are snapshot-able
    if (!bdrv_all_can_snapshot(has_devices, devices, errp)) { return false; }

    BlockDriverState* bs = bdrv_all_find_vmstate_bs(vm_state, has_devices, devices, errp);
    if (bs == NULL)
    {
        ret =  false;
        goto end;
    }


    vm_stop(RUN_STATE_SAVE_VM);


    // ─── Saving Drive Block ──────────────────────────────────────────────

    // Retrieve all snaspshot able drive
    g_autoptr(GList) bdrvs = NULL;
    if (bdrv_all_get_snapshot_devices(false, NULL, &bdrvs, errp)) {
        ret =  false;
        goto end;
    }

    // Trigger the global state to be saved within the snapshot
    global_state_store();

    GList* iterbdrvs = bdrvs;
    const char* base_bdrv_filename = NULL; //TODO transform this into g_string
    while (iterbdrvs) {
        bs = iterbdrvs->data;
        if (!bdrv_is_read_only(bs)) {
            ret = save_snapshot_external_bdrv(bs, snap_name, checkpoint_num, errp);

            if (!ret) goto end;
            base_bdrv_filename = get_base_bdrv(bs)->filename;
        }
        iterbdrvs = iterbdrvs->next;
    }

    // ─── Saving Main Memory ──────────────────────────────────────────────

    ret = save_snapshot_external_mem(base_bdrv_filename, snap_name,
                                     checkpoint_num, errp);
    if (!ret) goto end;

#ifdef CONFIG_LIBQFLEX
    if (qemu_libqflex_state.is_initialised) {
        g_autoptr(GString) dst_path = g_string_new("");

        g_autofree char* base_bdrv_dirname = g_path_get_dirname(base_bdrv_filename);
        g_string_append_printf(dst_path, "%s/%s", base_bdrv_dirname, snap_name);

        if (checkpoint_num != -1)
            g_string_append_printf(dst_path, "/checkpoint_%i", checkpoint_num);

        // call flexus API to trigger the cache memory dump
        flexus_api.qmp(QMP_FLEXUS_DOSAVE, dst_path->str);
    }
#endif

end:

    //? Allow to start the vm again only if no error was detected
    if (!ret)
        error_report_err(*errp);
    else
        vm_resume(saved_state);

    return ret;
}
