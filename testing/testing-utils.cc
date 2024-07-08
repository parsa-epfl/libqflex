#include "testing-utils.hh"

uint
rand_iters(uint width)
{
  if (width < 10) {
    return 1 << width;
  } else {
    return 1 << (width / 2);
  }
}

uint
rand_bits(uint width)
{
  return rand() & ((1 << width) - 1);
}
