#include "qemu/osdep.h"

#include "qemu/error-report.h"
#include "qapi/qapi-commands-middleware.h"
#include "trace.h"


void qmp_savevm_external(Error **errp)
{
    printf("Hello, world!\n");
}

void qmp_loadvm_external(Error **errp)
{
    printf("Hello, world!\n");
}