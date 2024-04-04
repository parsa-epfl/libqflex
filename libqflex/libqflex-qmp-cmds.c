#include "qemu/osdep.h"

#include "middleware/trace.h"
#include "qemu/error-report.h"
#include "qapi/qapi-commands-middleware.h"


void qmp_flexus_save_measure(const char *path, Error **errp)
{
    printf("Hello, world!\n");
}
