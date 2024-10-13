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

        if (qemu_snapvm_state.is_save_enabled)
        {
            qemu_log("> [SnapVM] Save External Memory enabled\n");
        }

        if (qemu_snapvm_state.is_load_enabled)
        {
            qemu_log("> [SnapVM] Load External Memory enabled\n");
            qemu_log("> [SnapVM] NAME       =%s\n", qemu_snapvm_state.loadvm_name);
            qemu_log("> [SnapVM] LOAD_PATH  =%s\n", qemu_snapvm_state.loadvm_path);
        }
}

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
