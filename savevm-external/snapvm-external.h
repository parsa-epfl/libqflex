#ifndef SNAPVM_EXT_H
#define SNAPVM_EXT_H

#include "block/block_int-io.h"



// ─── Module ──────────────────────────────────────────────────────────────────


struct snapvm_external_state {
    // True if the VM has been started with either -savevm|loadvm-external
    bool is_enabled;
    // True if the VM has loaded an image through -loadvm-external
    bool has_been_loaded;
};

extern QemuOptsList qemu_savevm_external_opts;
extern struct libqflex_state_t qemu_libqflex_state;

extern struct snapvm_external_state qemu_snapvm_ext_state;

typedef struct SnapTransaction
{

    //Type of save
    enum {
        INCREMENT,
        NEW_ROOT,
    } mode;

    // Datetime for increment snap
    char* datetime;

    char const * new_name;

    struct {
        // Pointer to the drive block
        BlockDriverState* bs;
        char* fullpath;
        char* dirname;
        char* basename;
    } root_bdrv;

    struct {
        // Pointer to the drive block
        BlockDriverState* bs;
        char* fullpath;
        char* dirname;
        char* basename;
    } new_bdrv;


} SnapTransaction;

// ─── Common ──────────────────────────────────────────────────────────────────


void
snapvm_init(
    QemuOpts* opts,
    char const * const loadvm,
    Error** errp);

char* get_datetime(void);


// ─── Save ────────────────────────────────────────────────────────────────────

bool
save_snapshot_external(
    SnapTransaction* trans,
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