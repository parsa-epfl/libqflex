#include "qemu/osdep.h"

#include "middleware/trace.h"
#include "qapi/qapi-commands-middleware.h"
#include "qemu/error-report.h"

void qmp_savevm_external(Error **errp) { printf("Hello, world!\n"); }

void qmp_loadvm_external(Error **errp) { printf("Hello, world!\n"); }
