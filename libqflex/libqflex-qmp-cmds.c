#include "qemu/osdep.h"

#include "middleware/trace.h"
#include "qapi/qapi-commands-middleware.h"
#include "qemu/error-report.h"

void qmp_flexus_save_measure(const char *path, Error **errp)
{
        printf("Hello, world!\n");
}

void qmp_flexus_save_ckpt(const char *path, Error **errp)
{
        printf("Hello, world!\n");
}
