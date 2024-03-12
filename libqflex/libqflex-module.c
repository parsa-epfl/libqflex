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

vCPU_t* libqflex_vcpus;
FLEXUS_API_t flexus_api;

// ─── Global Variable ─────────────────────────────────────────────────────────

QemuOptsList qemu_libqflex_opts = {
    .name = "libqflex",
    .merge_lists = true,
    .head = QTAILQ_HEAD_INITIALIZER(qemu_libqflex_opts.head),
    .desc = {
        {
            .name = "lib-path",
            .type = QEMU_OPT_STRING,
        },
        {
            .name = "cfg-path",
            .type = QEMU_OPT_STRING,

        },
        { /* end of list */ }
    },
};


struct libqflex_state_t qemu_libqflex_state = {
    .n_vcpus        = 0,
    .is_configured  = false,
    .is_initialised = false,
    .lib_path       = "",
    .cfg_path       = "",
};

// ─── Local Variable ──────────────────────────────────────────────────────────


// ─── Static Function ─────────────────────────────────────────────────────────





// ─────────────────────────────────────────────────────────────────────────────

/**
 * Retrieve pointer to CPU State and CPU Arch State from the global memory.
 * This avoid to retrieve and loop over cpu for every register access.
 *
 * Bryan Perdrizat
 *      Here we prefer accessing vCPU though their index for simplicity,
 *      despite that there exists other access method, like CPU_FOREACH(),
 *      and that it rely on the -smp parsing which may set thread/socket instead
 *      of cpus.
 *
 * @param n_vcpu Number of vCPU to retrieve.
 */
static void
libqflex_populate_vcpus(size_t n_vcpu)
{
    libqflex_vcpus = g_new0(vCPU_t, n_vcpu);

    for(size_t i = 0; i < n_vcpu; i++)
    {
        libqflex_vcpus[i].index = i;

        libqflex_vcpus[i].state = qemu_get_cpu(i);
        g_assert(libqflex_vcpus[i].state != NULL);

        libqflex_vcpus[i].env = cpu_env(libqflex_vcpus[i].state);
        g_assert(libqflex_vcpus[i].env != NULL);

        libqflex_vcpus[i].cpu = ARM_CPU(libqflex_vcpus[i].state);
        g_assert(libqflex_vcpus[i].cpu != NULL);
    }

    qemu_log("> [Libqflex] Populated %zu cpu(s)\n", n_vcpu);
}

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
    qemu_log("> [Libqflex] Init\n");

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


    // libqflex_trace_init();



    qemu_libqflex_state.is_initialised = true;
    qemu_log("> [Libqflex] PC=%llx \n", libqflex_vcpus[0].env->pc);
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

    char const * const lib_path = qemu_opt_get(opts, "lib-path");
    char const * const cfg_path = qemu_opt_get(opts, "cfg-path");

    if (lib_path) qemu_libqflex_state.lib_path = strdup(lib_path);
    if (cfg_path) qemu_libqflex_state.cfg_path = strdup(cfg_path);

    qemu_opts_del(opts);

    qemu_libqflex_state.is_configured = true;
    qemu_log("> [Libqflex] Configuration\n");
    qemu_log("> [Libqflex] LIB_PATH     =%s\n", qemu_libqflex_state.lib_path);
    qemu_log("> [Libqflex] CFG_PATH     =%s\n", qemu_libqflex_state.cfg_path);
}
