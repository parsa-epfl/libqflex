#ifndef SAVEVM_EXT_H
#define  SAVEVM_EXT_H

bool
save_snapshot_external(
    const char *name,       // Snapshot output name
    bool overwrite,         // Delete existing snapshot
    const char *vmstate,
    bool has_devices,
    strList *devices,
    Error **errp);

void save_snapshot_external_mem(void);
void save_snapshot_external_bdrv(void);

void load_snapshot_external(void);
void load_snapshot_external_mem(void);
void load_snapshot_external_bdrv(void);

#endif