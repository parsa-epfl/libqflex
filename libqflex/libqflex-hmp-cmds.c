#include "qemu/osdep.h"

#include "libqflex-module.h"
#include "libqflex.h"
#include "middleware/trace.h"
#include "monitor/hmp.h"
#include "monitor/monitor.h"
#include "qapi/qmp/qdict.h"
#include "qemu/error-report.h"
#include "qemu/log.h"

void hmp_flexus_save_measure(Monitor *mon, const QDict *qdict)
{
        if (!qemu_libqflex_state.is_configured) {
                monitor_printf(
                    mon,
                    "Please activate `libqflex' to use flexus QMP commands.\n");
                return;
        }

        Error *err = NULL;

        flexus_api.qmp(QMP_FLEXUS_SAVESTATS, "all.stats.out");
        flexus_api.qmp(QMP_FLEXUS_WRITEMEASUREMENT, "all:all.measurement.out");

        hmp_handle_error(mon, err);
}

void hmp_flexus_save_ckpt(Monitor *mon, const QDict *qdict)
{
        if (!qemu_libqflex_state.is_configured) {
                monitor_printf(mon, "qflex seems to not be initialised, saving "
                                    "flexus make then no sense\n");
                return;
        }

        Error *err = NULL;

        // Memory leak here
        char const *dst_path;

        if (!qdict_get_try_str(qdict, "dirname")) {
                g_autoptr(GDateTime) now = g_date_time_new_now_local();
                dst_path =
                    g_date_time_format(now, "checkpoint/%Y_%m_%d-%H%M_%S");
        } else {
                dst_path = qdict_get_try_str(qdict, "dirname");
        }

        libqflex_save_ckpt(dst_path);

        hmp_handle_error(mon, err);
}

void hmp_flexus_load_ckpt(Monitor *mon, const QDict *qdict)
{
        if (!qemu_libqflex_state.is_configured) {
                monitor_printf(mon, "qflex seems to not be initialised, "
                                    "loading to flexus make then no sense\n");
                return;
        }

        Error *err = NULL;

        // TODO: Check that the directory exists
        char const *const folder_name = qdict_get_try_str(qdict, "dirname");

        libqflex_load_ckpt(folder_name);

        hmp_handle_error(mon, err);
}
