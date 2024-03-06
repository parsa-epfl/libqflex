#ifndef SNAPVM_EXT_H
#define SNAPVM_EXT_H

// ─── Module ──────────────────────────────────────────────────────────────────


struct snapvm_external_state {
    bool is_enabled;

};

extern QemuOptsList qemu_savevm_external_opts;
extern struct libqflex_state_t qemu_libqflex_state;

extern struct snapvm_external_state qemu_snapvm_ext_state;


// ─── Common ──────────────────────────────────────────────────────────────────


void
snapvm_init(
    QemuOpts* opts,
    char const * const loadvm,
    Error** errp);

void
join_datetime(
    GString* export_path,
    char const * const filename);

void
create_snapshot_directory(
    GString* export_path,
    char const * const base_bdrv_filename,
    char const * const snap_name,
    int checkpoint_num);


// ─── Save ────────────────────────────────────────────────────────────────────



bool
save_snapshot_external(
    const char* snap_name,    // Snapshot output name
    const char* vm_state,
    bool has_devices,
    strList* devices,
    int checkpoint_num,
    Error** errp);


// ─── Load ────────────────────────────────────────────────────────────────────


void
loadvm_external_create_overlay(
    const char* snap_name,
    const char* base_bdrv_filename,
    const char* fmt,
    GString* overlay_path,
    Error** errp);

bool
load_snapshot_external(void);

#endif