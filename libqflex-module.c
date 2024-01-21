

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <glib.h>

#include "qemu/typedefs.h"
#include "qemu/option.h"
// #include "target/arm/kvm-consts.h"
// #include "qemu/compiler.h"

#include "libqflex-module.h"

QemuOptsList qemu_libqflex_opts = {
    .name = "simulator",
    .merge_lists = true,
    .head = QTAILQ_HEAD_INITIALIZER(qemu_libqflex_opts.head),
    .desc = {
        { /* end of list */ }
    },
};

void 
libqflex_init(void)
{
    printf("Initialised LIBQFLEX !!");
}