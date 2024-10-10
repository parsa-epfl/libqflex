#ifndef SNAPVM_EXT_H
#define SNAPVM_EXT_H

#include "block/block_int-io.h"

// ─── Module ──────────────────────────────────────────────────────────────────

struct snapvm_state_t {
        // Name of the checkpoint.
        // This should be the same between the external memory filename, and the .qcow2 snapshot name.
        char const * loadvm_name;
        // Path were the checkpoint have to be found
        // This should be the same between the external memory filename, and the .qcow2 snapshot name.
        char const * loadvm_path;

        // True if the VM has been started with either -savevm|loadvm-external
        bool is_save_enabled;
        bool is_load_enabled;
        // True if the VM has loaded an image through -loadvm-external
        bool has_been_loaded;
};

extern QemuOptsList qemu_snapvm_loadvm_opts;
extern struct libqflex_state_t qemu_libqflex_state;
extern struct snapvm_state_t qemu_snapvm_state;

// ─── Common ──────────────────────────────────────────────────────────────────

void
snapvm_loadvm_parse_opts(char const *optarg);

void
snapvm_init(void);

char *get_datetime(void);

// ─── Save ────────────────────────────────────────────────────────────────────

//bool save_snapshot_external(SnapTransaction *trans, Error **errp);

// ─── Load ────────────────────────────────────────────────────────────────────

bool load_snapshot_external(const char *name, const char *vmstate,
                            bool has_devices, strList *devices, Error **errp);

#endif
