#ifndef TESTING_UTILS_HH
#define TESTING_UTILS_HH

#include <stdlib.h>

/*
  * size: 00 (8-bit), 01 (16-bit), 10 (32-bit), 11 (64-bit), 100 (128-bit)
  * accesses: ?
*/

uint
rand_iters(uint width);

uint
rand_bits(uint width);

#endif
