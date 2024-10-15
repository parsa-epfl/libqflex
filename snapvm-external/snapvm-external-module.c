#include "qemu/osdep.h"

#include "snapvm-external.h"

#include "qapi/error.h"
#include "qemu/config-file.h"
#include "qemu/option.h"
#include "qemu/log.h"

// ─── Global Variable ─────────────────────────────────────────────────────────

struct snapvm_state_t qemu_snapvm_state = {
    .name = "",
    .path = "",
    .is_save_enabled = false,
    .is_load_enabled = false,
};

// ─────────────────────────────────────────────────────────────────────────────
void
snapvm_external_parse_opts(char const *optarg)
{

        if (optarg == NULL)
                exit(EXIT_FAILURE);

        qemu_snapvm_state.path = strdup(optarg);
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
            qemu_log("> [SnapVM] SAVE_PATH  =%s\n", qemu_snapvm_state.path);
        }

        if (qemu_snapvm_state.is_load_enabled)
        {
            g_assert(qemu_snapvm_state.is_save_enabled); // SnapVM should have been called as argument to inidicate the path for external snapshot

            g_autofree char* load_path = g_build_filename(qemu_snapvm_state.path, qemu_snapvm_state.name, NULL);
            qemu_log("> [SnapVM] Load External Memory enabled\n");
            qemu_log("> [SnapVM] NAME       =%s\n", qemu_snapvm_state.name);
            qemu_log("> [SnapVM] LOAD_PATH  =%s\n", load_path);

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
