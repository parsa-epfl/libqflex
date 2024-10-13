#include "qemu/osdep.h"

#include "middleware/trace.h"

#include "monitor/hmp.h"
#include "monitor/monitor.h"
#include "qapi/qmp/qdict.h"
#include "qemu/error-report.h"
#include "qemu/log.h"
#include "snapvm-external.h"
#include "sysemu/runstate.h"

void hmp_loadvm_external(Monitor *mon, const QDict *qdict)
{
        Error *err = NULL;
        // RunState saved_state    = runstate_get();
        // __attribute_maybe_unused__  const char* name = qdict_get_str(qdict,
        // "name");

        vm_stop(RUN_STATE_SAVE_VM);

        // if (load_snapshot_external(/*name, NULL, false, NULL, -1, &err*/))
        //     vm_resume(saved_state);

        hmp_handle_error(mon, err);
}

void hmp_savevm_external(Monitor *mon, const QDict *qdict)
{
        Error *err = NULL;
        RunState saved_vm_running = runstate_is_running();

        const char* name = qdict_get_str(qdict, "name");

        vm_stop(RUN_STATE_SAVE_VM);
        save_snapshot_external(name, &err);

        if (saved_vm_running)
            vm_start();

        hmp_handle_error(mon, err);
}
