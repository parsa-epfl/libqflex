#ifndef LIBQFLEX_MODULE_H
#define LIBQFLEX_MODULE_H

#include "libqflex-legacy-api.h"

typedef void (*FLEXUS_INIT_t)(QEMU_API_t *, FLEXUS_API_t *, int, const char *,
                              const char *, const char *, const char *);

extern QemuOptsList qemu_libqflex_opts;

struct libqflex_state_t {

        size_t n_vcpus;

        bool is_configured;
        bool is_running;

        char const *lib_path;
        char const *cfg_path;
        char const *ckpt_path;
        char const *debug_lvl;

        uint32_t cycles;
        uint32_t cycles_mask;

        enum {
                MODE_TRACE,
                MODE_TIMING,
        } mode;
};

/**
 * Entry point of libqflex. This is called directly in qemu_init().
 * The function is not self-aware of QEMU' state and -libqflex flags,
 * therefore the state is kept within a file scoped structure.
 */
void libqflex_init(void);

/**
 * CLI argument parser. This is called to parse libqflex args during QEMU
 * initialisation phase.
 */
void libqflex_parse_opts(char const *optarg);

/**
 * Return true if the mode is timing and it has been sucessfully initialised
 * Used by the TCG to check if flexus can start singlestepping.
 */
bool libqflex_is_timing_ready(void);

#endif