#include "qemu/osdep.h"

#include "monitor/hmp.h"
#include "monitor/monitor.h"
#include "qapi/qmp/qdict.h"
#include "qemu/error-report.h"
#include "qemu/log.h"
#include "sysemu/runstate.h"

#include "qapi/qmp/qdict.h"

#include "middleware/trace.h"
#include "libqflex-module.h"

extern struct libqflex_state_t qemu_libqflex_state;

void
hmp_flexus_save_measure(Monitor *mon, const QDict *qdict) {
    if (! qemu_libqflex_state.is_configured)
    {
        monitor_printf(mon, "Please activate `libqflex' to use flexus QMP commands.\n");
        return;
    }

    Error* err = NULL;

    flexus_api.qmp(QMP_FLEXUS_SAVESTATS, "all.stats.out");
    flexus_api.qmp(QMP_FLEXUS_WRITEMEASUREMENT, "all:all.measurement.out");

    hmp_handle_error(mon, err);
}

void
hmp_flexus_save_ckpt(Monitor* mon, const QDict* qdict)
{
    if (! qemu_libqflex_state.is_configured)
    {
        monitor_printf(mon, "qflex seems to not be initialised, saving flexus make then no sense\n");
        return;
    }

    Error    *err           = NULL;
    int saved_vm_running = runstate_is_running();
    vm_stop(RUN_STATE_SAVE_VM);

    flexus_api.qmp(QMP_FLEXUS_DOSAVE, "./flexus_ckpt");

    if (saved_vm_running)
        vm_start();

    hmp_handle_error(mon, err);
}

void
hmp_flexus_load_ckpt(Monitor* mon, const QDict* qdict)
{
    if (! qemu_libqflex_state.is_configured)
    {
        monitor_printf(mon, "qflex seems to not be initialised, loading to flexus make then no sense\n");
        return;
    }

    Error    *err           = NULL;
    int saved_vm_running = runstate_is_running();
    vm_stop(RUN_STATE_SAVE_VM);

    // TODO: Check that the directory exists
    const char *folder_name = qdict_get_try_str(qdict, "dirname");

    flexus_api.qmp(QMP_FLEXUS_DOLOAD, folder_name);

    if (saved_vm_running)
        vm_start();

    hmp_handle_error(mon, err);
}