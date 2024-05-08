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
hmp_flexus_save_measure(Monitor *mon, const QDict *qdict)
{
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
        monitor_printf(mon, "QFlex seems to not be initialised, saving Flexus make then no sense\n");
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