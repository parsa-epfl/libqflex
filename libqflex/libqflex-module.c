 #include <dlfcn.h>

#include "qemu/osdep.h"

#include "hw/boards.h"
#include "qapi/error.h"
#include "qemu/config-file.h"
#include "qemu/error-report.h"
#include "qemu/log.h"
#include "qemu/option.h"
#include "sysemu/tcg.h"

#include "libqflex-legacy-api.h"
#include "libqflex-module.h"
#include "libqflex.h"
#include "plugins/trace/trace.h"



// ─────────────────────────────────────────────────────────────────────────────

FLEXUS_API_t flexus_api = {NULL};

// ─── Global Variable ─────────────────────────────────────────────────────────

QemuOptsList qemu_libqflex_opts = {
    .name = "libqflex",
    .merge_lists = true,
    .head = QTAILQ_HEAD_INITIALIZER(qemu_libqflex_opts.head),
    .desc = {
        {
            .name = "mode",
            .type = QEMU_OPT_STRING,

        },
        {
            .name = "lib-path",
            .type = QEMU_OPT_STRING,
        },
        {
            .name = "cfg-path",
            .type = QEMU_OPT_STRING,

        },
        {
            .name = "cycles",
            .type = QEMU_OPT_NUMBER,

        },
        {
            .name = "cycles-mask",
            .type = QEMU_OPT_NUMBER,

        },
        {
            .name = "debug",
            .type = QEMU_OPT_STRING,

        },
        { /* end of list */ }
    },
};


struct libqflex_state_t qemu_libqflex_state = {
    .n_vcpus        = 0,
    .is_configured  = false,
    .is_running = false,
    .lib_path       = "",
    .cfg_path       = "",
    .cycles         = 0,
    .cycles_mask    = 0,
    .debug_lvl      = "vverb",
    .mode           = MODE_TRACE,
};

// ─── Local Variable ──────────────────────────────────────────────────────────


// ─── Static Function ─────────────────────────────────────────────────────────



static bool
libqflex_flexus_init(void)
{
    if (!qemu_libqflex_state.lib_path)
    {
        error_report("ERROR: missing lib-path");
        return false;
    }

    if (!qemu_libqflex_state.cfg_path)
    {
        error_report("ERROR: missing cfg-path");
        return false;
    }

    void* handle = NULL;
    if ((handle = dlopen(qemu_libqflex_state.lib_path, RTLD_LAZY)) == NULL)
    {
        error_report("ERROR: while opening %s => %s", qemu_libqflex_state.lib_path, dlerror());
        return false;
    }

    FLEXUS_INIT_t flexus = NULL;
    if ((flexus = (FLEXUS_INIT_t)(dlsym(handle, "flexus_init"))) == NULL)
    {
        error_report("ERROR: cannot find 'flexus_init' in %s: %s", qemu_libqflex_state.lib_path, dlerror());
        return false;
    }

    QEMU_API_t qemu_api =
    {
        .read_register      = libqflex_read_register,
        .get_num_cores      = libqflex_get_nb_cores,
        .translate_va2pa    = libqflex_translate_VA,
        .get_pc             = libqflex_get_pc,
        .has_irq            = libqflex_has_interrupt,
        .cpu_exec           = libqflex_advance,
    };

    // Flexus is stupid, so it's to put with its stupidity
    g_autoptr(GString) nb_cycles = g_string_new("");
    g_string_printf(nb_cycles, "%d", qemu_libqflex_state.cycles);

    flexus(
        &qemu_api, &flexus_api,
        qemu_libqflex_state.n_vcpus,
        qemu_libqflex_state.cfg_path,
        qemu_libqflex_state.debug_lvl,
        nb_cycles->str,
        "." // CWD
    );

    return true;
}

// ─────────────────────────────────────────────────────────────────────────────

void
libqflex_init(void)
{
    // Make sure that we have parsed the cli options
    if (! qemu_libqflex_state.is_configured)
        return;

    bool ret = true;


    if (! tcg_enabled()) {
        error_report("ERROR: TCG must be enabled");
        exit(EXIT_FAILURE);
    }

    //? `current_machine` accessible only after options parsing
    qemu_libqflex_state.n_vcpus = current_machine->smp.cpus;
    // libqflex_api_init();
    libqflex_populate_vcpus(qemu_libqflex_state.n_vcpus);

    ret = libqflex_flexus_init();
    if (!ret) exit(EXIT_FAILURE);

    if (qemu_libqflex_state.mode == MODE_TRACE)
        libqflex_trace_init();

    qemu_libqflex_state.is_running = true;
    qemu_log("> [Libqflex] Init\n");
    qemu_log("> [Libqflex] LIB_PATH     =%s\n", qemu_libqflex_state.lib_path);
    qemu_log("> [Libqflex] CFG_PATH     =%s\n", qemu_libqflex_state.cfg_path);
    qemu_log("> [Libqflex] CYCLES       =%d\n", qemu_libqflex_state.cycles);
    qemu_log("> [Libqflex] DEBUG        =%s\n", qemu_libqflex_state.debug_lvl);
}

// ─────────────────────────────────────────────────────────────────────────────

void
libqflex_parse_opts(char const * optarg)
{
    QemuOpts* opts = qemu_opts_parse_noisily(
        qemu_find_opts("libqflex"),
        optarg,
        false);

    if (opts == NULL)
        exit(EXIT_FAILURE);

    char const * const mode = qemu_opt_get(opts, "mode");
    char const * const lib_path = qemu_opt_get(opts, "lib-path");
    char const * const cfg_path = qemu_opt_get(opts, "cfg-path");
    char const * const debug_lvl = qemu_opt_get(opts, "debug");
    uint32_t const cycles       = qemu_opt_get_number(opts, "cycles", 0);
    uint32_t const cycles_mask  = qemu_opt_get_number(opts, "cycles-mask", 1);

    qemu_libqflex_state.cycles = cycles;
    qemu_libqflex_state.cycles_mask = cycles_mask;

    if (lib_path) qemu_libqflex_state.lib_path = strdup(lib_path);
    if (cfg_path) qemu_libqflex_state.cfg_path = strdup(cfg_path);
    if (debug_lvl) qemu_libqflex_state.debug_lvl = strdup(debug_lvl);

    if (mode)
    {
        if (strcmp(strdup(mode), "trace") == 0)  qemu_libqflex_state.mode = MODE_TRACE;
        if (strcmp(strdup(mode), "timing") == 0) qemu_libqflex_state.mode = MODE_TIMING;
    }

    qemu_opts_del(opts);

    qemu_libqflex_state.is_configured = true;
}


bool
libqflex_is_timing_ready(void)
{
    return qemu_libqflex_state.is_running &&
            (qemu_libqflex_state.mode == MODE_TIMING);
}
