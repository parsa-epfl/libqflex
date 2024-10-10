#include "qemu/osdep.h"

#include "snapvm-external.h"

#include "qapi/error.h"
#include "qemu/config-file.h"
#include "qemu/option.h"
#include "qemu/log.h"

// ─── Global Variable ─────────────────────────────────────────────────────────

QemuOptsList qemu_snapvm_loadvm_opts = {
    .name = "loadvm-external",
    .merge_lists = true,
    .head = QTAILQ_HEAD_INITIALIZER(qemu_snapvm_loadvm_opts.head),
    .desc = {{
                 .name = "name",
                 .type = QEMU_OPT_STRING,

             },
             {
                 .name = "path",
                 .type = QEMU_OPT_STRING,
             },
             {/* end of list */}},
};

struct snapvm_state_t qemu_snapvm_state = {
    .loadvm_name = "",
    .loadvm_path = "",
    .is_save_enabled = false,
    .is_load_enabled = false,
    .has_been_loaded = false,
};

// ─────────────────────────────────────────────────────────────────────────────

void
snapvm_loadvm_parse_opts(char const *optarg)
{
        QemuOpts *opts = qemu_opts_parse_noisily(qemu_find_opts("loadvm-external"), optarg, false);

        if (opts == NULL)
                exit(EXIT_FAILURE);

        char const *const name = qemu_opt_get(opts, "name");
        char const *const loadvm_path = qemu_opt_get(opts, "path");

        if (name)
                qemu_snapvm_state.loadvm_name = strdup(name);
        if (loadvm_path)
                qemu_snapvm_state.loadvm_path = strdup(loadvm_path);

        qemu_opts_del(opts);

        qemu_snapvm_state.has_been_loaded = true;

}
/**
 * Called on every block drive during system emulation starting phase.
 * This look for the 'readonly' flag, if it exist, it make sure that
 * the flag is turned 'off'. The function alter the 'file=' attribut
 * and replaces it with a new filename.
 */
void
snapvm_init(void)
{
        qemu_log("> [SnapVM] Init\n");
        qemu_log("> [SnapVM] NAME       =%s\n", qemu_snapvm_state.loadvm_name);
        qemu_log("> [SnapVM] LOAD_PATH  =%s\n", qemu_snapvm_state.loadvm_path);
}
//{
//     const char *readonly = qemu_opt_get(opts, "readonly");
//
//     if (readonly && !g_str_equal(readonly, "off"))
//         return;
//
//
//     const char* file   = qemu_opt_get(opts, "file");
//     const char* format = qemu_opt_get(opts, "format");
//     // default to qcow2 if no flag
//     format = format ?: "qcow2";
//
//
//
//     // load the new overlay from last backed image
//     g_assert(g_file_test(file, G_FILE_TEST_IS_REGULAR));
//     g_autoptr(GString) overlay = g_string_new("");
//     loadvm_external_create_overlay(loadvm, file, format, overlay, errp);
//
//     // open the newly created overlay to not require lock of base image
//     g_assert(overlay->len > 0); //? making sure the overlay is not an empty
//     string qemu_opt_set(opts, "file", overlay->str, errp);
// }

// ─── Common ──────────────────────────────────────────────────────────────────

/**
 * Append the datetime to a filename (string)
 * Return like -> 2024_03_06-1717_34
 *
 * RETURN must be freed by the caller
 */
char *get_datetime(void)
{
        g_autoptr(GDateTime) now = g_date_time_new_now_local();

        return g_date_time_format(now, "%Y_%m_%d-%H%M_%S");
}
