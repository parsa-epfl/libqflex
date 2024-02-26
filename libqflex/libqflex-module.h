#ifndef LIBQFLEX_MODULE_H
#define LIBQFLEX_MODULE_H


extern QemuOptsList qemu_libqflex_opts;

struct libqflex_state_t {
    size_t n_vcpus;

    bool is_initialised;
    bool is_configured;

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
void libqflex_configure(void);

#endif