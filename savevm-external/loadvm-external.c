#include "qemu/osdep.h"

#include "monitor/hmp.h"
#include "monitor/monitor.h"
#include "qapi/error.h"
#include "qapi/qmp/qdict.h"
#include "qemu/error-report.h"
#include "qemu/log.h"
#include "sysemu/runstate.h"

#include "snapvm-external.h"

bool
load_snapshot_external(void)
{
    return true;
}

/**
 * When -savevm-external is enabled create a tmp disk if to avoid making
 * change to the base drive.
 * @attribut bdrv_filename: path for attribut -drive file=""
 */
void
loadvm_external_create_overlay(
    const char* snap_name,
    const char* file,
    const char* fmt,
    GString* overlay_path,
    Error** errp)
{
    // g_autoptr(GString) snap_path_dst = g_string_new("");
    g_autoptr(GString) snap_fullpath_tmp = g_string_new("");
    g_autoptr(GString) snap_filename_tmp = g_string_new("");

    /**
     * Handle case 1: No snap_name, create a tmp directory, and copy the root image
     * TODO: handle case when a snap name is present
     */

    //? Construct full path
    g_autofree char* datetime = get_datetime();
    g_autofree char* bdrv_dirname   = g_path_get_dirname(file);
    g_autofree char* bdrv_filename  = g_path_get_basename(file);

    g_string_append_printf(snap_filename_tmp, "%s-%s", bdrv_dirname, datetime);

    //? Construct a path like [previous_disk_path]/tmp/[new disk_name.format]
    g_string_append_printf(
        snap_fullpath_tmp,
        "%s/tmp/%s",
        bdrv_dirname,
        snap_filename_tmp->str);

    // Get the dirname of the full path and create remaining folder
    g_mkdir_with_parents(g_path_get_dirname(snap_fullpath_tmp->str), 0700);

    // Create a block drive (bdrv) at {snap_fullpath_tmp} with backing file at {file}
    bdrv_img_create(snap_fullpath_tmp->str, fmt, file, fmt, NULL, -1, 0, false, errp);
    g_assert(g_file_test(snap_fullpath_tmp->str, G_FILE_TEST_IS_REGULAR));
    /**
     * The tmp file become the overlay, and will replace the original string
     * in the option. Copying the string now avoid any crazy behaviour where
     * a previous function modifiy the string without us noticing
     */

    g_string_append(overlay_path, snap_fullpath_tmp->str);

    //FIXME: WHY error_report_err(*errp);
}
