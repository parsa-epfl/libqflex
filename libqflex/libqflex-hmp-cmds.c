#include "qemu/osdep.h"

#include "monitor/hmp.h"
#include "monitor/monitor.h"
#include "qapi/qmp/qdict.h"
#include "qemu/error-report.h"
#include "qemu/log.h"

#include "qapi/qmp/qdict.h"

#include "middleware/trace.h"
#include "libqflex-module.h"
#include "libqflex.h"

// add these?

#include "middleware/libqflex/plugins/trace/trace.h"
#include "libqflex-module.h"
#include "libqflex.h"
#include "qapi/qmp/qdict.h"
#include "qapi/qmp/qobject.h"

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

    Error* err = NULL;

    // Memory leak here
    char const * dst_path;

    if(!qdict_get_try_str(qdict, "dirname"))
    {
        g_autoptr(GDateTime) now = g_date_time_new_now_local();
        dst_path = g_date_time_format(now, "checkpoint/%Y_%m_%d-%H%M_%S");
    }
    else
    {
        dst_path = qdict_get_try_str(qdict, "dirname");
    }

    libqflex_save_ckpt(dst_path);

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

    Error* err = NULL;

    // TODO: Check that the directory exists
    char const * const folder_name = qdict_get_try_str(qdict, "dirname");

    libqflex_load_ckpt(folder_name);

    hmp_handle_error(mon, err);
}

// Functions to call periodic checkpoint

// Structure for Monitor and QDict
typedef struct {
    Monitor *mon;
    const QDict *qdict;
    int checkpoint_interval;
    int sample_size;
} FlexusCkptData;

static QEMUTimer *flexus_ckpt_timer = NULL;
static void flexus_ckpt_timer_cb(void *opaque);  // Timer
static int last_instruction_count = 0;

static void flexus_ckpt_timer_cb(void *opaque) {
    FlexusCkptData *data = (FlexusCkptData *)opaque;
    int current_instruction_count = instruction_count;
    int instruction_diff = current_instruction_count - last_instruction_count;

    // Cannot trigger the checkpoint generation for every instruction,
    // so approximate this by checking the instruction count by every given second.
    // Currently it checks the instruction count every 10000 ms but this is adjustable.
    if (instruction_diff >= data->checkpoint_interval && current_instruction_count <= data->sample_size * data->checkpoint_interval + 0.5 * data->checkpoint_interval) { 
        // keep generating checkpoint until it reaches the target sample_size
        hmp_flexus_save_ckpt(data->mon, data->qdict);
        last_instruction_count = current_instruction_count;
    }

    // Reschedule the timer to run again in 10 seconds.
    timer_mod(flexus_ckpt_timer, qemu_clock_get_ms(QEMU_CLOCK_REALTIME) + 10000);
}

void hmp_flexus_start_periodic_ckpt(Monitor *mon, const QDict *qdict) {
    static const int DEFAULT_CHECKPOINT_INTERVAL = 1000000;
    static const int DEFAULT_SAMPLE_SIZE = 30;

    if (!qemu_libqflex_state.is_configured) {
        monitor_printf(mon, "qflex seems to not be initialized, periodic checkpoint saving makes no sense\n");
        return;
    }

    if (flexus_ckpt_timer) {
        timer_del(flexus_ckpt_timer);
        flexus_ckpt_timer = NULL;  // Reset the timer pointer
    }

    FlexusCkptData *data = g_malloc(sizeof(FlexusCkptData));
    data->mon = mon;
    data->qdict = qdict;

    // Retrieve optional arguments for checkpoint_interval and sample_size
    data->checkpoint_interval = qdict_haskey(qdict, "checkpoint_interval") ? qdict_get_int(qdict, "checkpoint_interval") : DEFAULT_CHECKPOINT_INTERVAL;
    data->sample_size = qdict_haskey(qdict, "sample_size") ? qdict_get_int(qdict, "sample_size") : DEFAULT_SAMPLE_SIZE;

    // Initialize the last_instruction_count
    last_instruction_count = instruction_count;

    flexus_ckpt_timer = timer_new_ms(QEMU_CLOCK_REALTIME, flexus_ckpt_timer_cb, data);
    timer_mod(flexus_ckpt_timer, qemu_clock_get_ms(QEMU_CLOCK_REALTIME) + 10000); // 10 seconds
}