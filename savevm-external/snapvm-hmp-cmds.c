#include "qemu/osdep.h"

#include "monitor/hmp.h"
#include "monitor/monitor.h"
#include "qapi/qmp/qdict.h"
#include "qemu/error-report.h"
#include "qemu/log.h"
#include "sysemu/runstate.h"

#include "qapi/qmp/qdict.h"

#include "snapvm-external.h"
#include "trace.h"

void
hmp_loadvm_external(Monitor *mon, const QDict *qdict)
{
    if (! qemu_snapvm_ext_state.is_enabled)
    {
        monitor_printf(mon, "Please activate `savevm-external' to use external snapshots.\n");
        return;
    }

    Error    *err           = NULL;
    RunState saved_state    = runstate_get();
    __attribute((maybe_unused))  const char* name = qdict_get_str(qdict, "name");

    vm_stop(RUN_STATE_SAVE_VM);

    // if (load_snapshot_external(/*name, NULL, false, NULL, -1, &err*/))
    //     vm_resume(saved_state);

    hmp_handle_error(mon, err);
}


void
hmp_savevm_external(Monitor* mon, const QDict* qdict)
{
    if (! qemu_snapvm_ext_state.is_enabled)
    {
        monitor_printf(mon, "Please activate `savevm-external' to use external snapshots.\n");
        return;
    }

    Error* err = NULL;

    SnapTransaction* trans = g_new0(SnapTransaction, 1);
    trans->new_name = qdict_get_try_str(qdict, "name");

    //? Only two mode so far but the idea is left out as a possibility to
    //? extend the mechinism further wuth new mode.
    trans->mode = (trans->new_name) ? NEW_ROOT : INCREMENT;


    // If there is a name is a new root
    save_snapshot_external(
        trans,
        &err
    );


    g_free(trans);
    hmp_handle_error(mon, err);
}