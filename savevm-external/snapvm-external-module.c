#include "qemu/osdep.h"

#include "qapi/error.h"
#include "qemu/option.h"

#include "snapvm-external.h"


// ─── Global Variable ─────────────────────────────────────────────────────────

QemuOptsList qemu_savevm_external_opts = {
    .name = "savevm-external",
    .merge_lists = false,
    .head = QTAILQ_HEAD_INITIALIZER(qemu_savevm_external_opts.head),
    .desc = {
        { /* end of list */ }
    },
};


struct snapvm_external_state qemu_snapvm_ext_state = {
    .is_enabled         = false,
    .has_been_loaded    = false,
};


// ─────────────────────────────────────────────────────────────────────────────


/**
 * Called on every block drive during system emulation starting phase.
 * This look for the 'readonly' flag, if it exist, it make sure that
 * the flag is turned 'off'. The function alter the 'file=' attribut
 * and replaces it with a new filename.
 */
void snapvm_init(QemuOpts* opts,  char const * const loadvm, Error** errp)
{
    const char *readonly = qemu_opt_get(opts, "readonly");

    if (readonly && !g_str_equal(readonly, "off"))
        return;


    const char* file   = qemu_opt_get(opts, "file");
    const char* format = qemu_opt_get(opts, "format");
    // default to qcow2 if no flag
    format = format ?: "qcow2";



    // load the new overlay from last backed image
    g_assert(g_file_test(file, G_FILE_TEST_IS_REGULAR));
    g_autoptr(GString) overlay = g_string_new("");
    loadvm_external_create_overlay(loadvm, file, format, overlay, errp);

    // open the newly created overlay to not require lock of base image
    g_assert(overlay->len > 0); //? making sure the overlay is not an empty string
    qemu_opt_set(opts, "file", overlay->str, errp);
}

// ─── Common ──────────────────────────────────────────────────────────────────

/**
 * Append the datetime to a filename (string)
 * Return like -> 2024_03_06-1717_34
 *
 * RETURN must be freed by the caller
 */
void
join_datetime(
    char* export_string,
    char const * const base_string,
    char* datetime)
{
    g_autoptr(GString)      new_filename_buf = g_string_new("");
    g_autoptr(GDateTime)    now              = g_date_time_new_now_local();

    //! Weird but okay
    datetime = g_date_time_format(now, "%Y_%m_%d-%H%M_%S");

    g_string_append_printf(new_filename_buf, "%s-%s", base_string, datetime);
    export_string = g_string_free(new_filename_buf, false);



}

// void
// create_dir_tree(const char* bdrv_filename)
// {

//     // absolute path or relative path
//     g_autofree char* dir_path   = g_path_get_dirname(bdrv_filename);
//     // Likely equal to image.qcow2 or rootfs.img
//     g_autofree char* filename   = g_path_get_basename(bdrv_filename);

//     // Assert both exist
//     g_assert(dir_path && filename);


// }

// void
// create_snapshot_directory(
//     GString* export_path,
//     char const * const base_bdrv_filename,
//     char const * const snap_name,
//     int checkpoint_num)
// {
//     g_autofree char* dir_path   = g_path_get_dirname(base_bdrv_filename);
//     g_autofree char* filename   = g_path_get_basename(base_bdrv_filename);

//     g_assert(dir_path && filename);

//     /**
//      * Create a new const string to get the snap name if needed,
//      * to avoid modifying trying to modify the NULL constant
//      */
//     char const * const folder_name = (snap_name == NULL) ? "tmp" : snap_name;
//     const char* format = checkpoint_num >= 0 ? "%s/%s/checkpoint_%i/" : "%s/%s/";

//     //! Need to check if this really work the way i think it does
//     g_string_append_printf(export_path, format, dir_path, folder_name, checkpoint_num);
//     // Always append the filename whatever append
//     g_string_append_printf(export_path, "%s", filename);
//     g_mkdir_with_parents(g_path_get_dirname(export_path->str), 0700);
// }
