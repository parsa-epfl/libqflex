#ifndef SAVEVM_EXT_H
#define  SAVEVM_EXT_H

bool
save_snapshot_external(
    const char* snap_name,    // Snapshot output name
    const char* vm_state,
    bool has_devices,
    strList* devices,
    int checkpoint_num,
    Error** errp);

extern struct libqflex_state_t qemu_libqflex_state;

// void save_snapshot_external_mem(void);
// void save_snapshot_external_bdrv(void);

void load_snapshot_external(void);
void load_snapshot_external_mem(void);
void load_snapshot_external_bdrv(void);

#endif