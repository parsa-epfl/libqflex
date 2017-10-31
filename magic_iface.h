#ifndef MAGIC_IFACE_H
#define MAGIC_IFACE_H

#include <stdio.h>

/* 
 * Function passes information to QEMU and Flexus
 * using magic instruction, triggering r30.
 * The info is passed through r0 (cmd_id), r1 and r2.
 */
void trigger_magic(uint64_t cmd_id, uint64_t user_v1, uint64_t user_v2)
{
   __asm__ __volatile__ (
       "mov x0, %[cmd_id] \n\t"
       "mov x1, %[user_reg1] \n\t"
       "mov x2, %[user_reg2] \n\t"
       "orr x30, x30, x30 \n\t"
       : /* no output */
       : [cmd_id] "r" (cmd_id), [user_v1] "r" (user_v1),
         [user_v2] "r" (user_v2) /* input regs */
       : "x0", "x1", "x2" /* clobbered regs */
   );
}

#endif // MAGIC_IFACE_H
