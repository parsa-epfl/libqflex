
// ─── Required Anyway ─────────────────────────────────────────────────────────

#include "qemu/osdep.h"

// ─── Required For Logging ────────────────────────────────────────────────────

#include <glib/gstdio.h>
#include "qapi/error.h"
#include "qemu/log.h"

// ─── Required For CPUs Abstraction ───────────────────────────────────────────

#include <glib/gmem.h>
#include <glib/gtestutils.h>
#include <glib/gmacros.h>

#include "hw/boards.h"
#include "libqflex.h"
#include "libqflex-module.h"
#include "plugins/trace/trace.h"
#include "legacy-qflex-api.h"

// ─── Required For Options ────────────────────────────────────────────────────

#include "qemu/option.h"


// ─────────────────────────────────────────────────────────────────────────────

FLEXUS_API_t flexus_api;
vCPU_t* libqflex_vcpus;
typedef void (*FLEXUS_INIT_t)(QEMU_API_t *, FLEXUS_API_t *, int, const char *, const char *, const char *, const char *);


// ─── Global Variable ─────────────────────────────────────────────────────────

QemuOptsList qemu_libqflex_opts = {
    .name = "libqflex",
    .merge_lists = false,
    .head = QTAILQ_HEAD_INITIALIZER(qemu_libqflex_opts.head),
    .desc = {
        { /* end of list */ }
    },
};


struct libqflex_state_t qemu_libqflex_state = {
    .n_vcpus = 0,
    .is_initialised = false,
    .is_configured = false,
};

// ─── Local Variable ──────────────────────────────────────────────────────────


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

// ─────────────────────────────────────────────────────────────────────────────

void
libqflex_init(void)
{
    if (! qemu_libqflex_state.is_configured)
        return;

    //! `current_machine` accessible only after options parsing
    qemu_libqflex_state.n_vcpus = current_machine->smp.cpus;

    libqflex_populate_vcpus(qemu_libqflex_state.n_vcpus);

    qemu_plugin_trace_init();


    // REGISTER HOOKS'n'Stuff
    qemu_libqflex_state.is_initialised = true;
    qemu_log("> [Libqflex] Init\n");

    // ─────────────────────────────────────────────────────────────────────

    qemu_log("> [Libqflex] PC=%lx \n", libqflex_vcpus[0].env->pc);
}

// ─────────────────────────────────────────────────────────────────────────────

void
libqflex_configure(void)
{
    // Ensure sane memory allocation
    qemu_libqflex_state.n_vcpus         = 0;
    qemu_libqflex_state.is_initialised  = false;
    qemu_libqflex_state.is_configured   = false;

    // PARSING here
    // ENVIRONEMENT CHECKING HERE (maybe)

    //? libqflex_init relies on the fact that -libqflex is part of the args.
    qemu_libqflex_state.is_configured = true;
    qemu_log("> [Libqflex] Config\n");
}
