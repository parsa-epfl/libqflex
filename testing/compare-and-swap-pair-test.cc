#include <gtest/gtest.h>
#include <stdlib.h>
#include "testing-utils.hh"

extern "C" {
  #include "../libqflex/plugins/trace/trace.h"
}

struct mem_access mem_access;

uint sz;
uint L;
uint o0;
uint Rt2 = 0b11111;

uint32_t opcode = (0b0 << 31) | (0b0010000 << 23) | (0b1 << 21) | (Rt2 << 10);

TEST(CompareAndSwapPair, CASP32Bit)
{
  sz = 0b0;
  L = 0b0;
  o0 = 0b0;

  opcode |= (sz << 30) | (L << 22) | (o0 << 15);

  EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, opcode), true);
  EXPECT_EQ(mem_access.is_load, true);
  EXPECT_EQ(mem_access.is_store, true); // conditional store?
  EXPECT_EQ(mem_access.is_vector, false);
  EXPECT_EQ(mem_access.is_signed, false);
  EXPECT_EQ(mem_access.is_pair, true);
  EXPECT_EQ(mem_access.is_atomic, true);
  EXPECT_EQ(mem_access.size, 0b10);
  EXPECT_EQ(mem_access.accesses, 1); // conditional store?
}


TEST(CompareAndSwapPair, CASPL32Bit)
{
  sz = 0b0;
  L = 0b0;
  o0 = 0b1;

  opcode |= (sz << 30) | (L << 22) | (o0 << 15);

  EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, opcode), true);
  EXPECT_EQ(mem_access.is_load, true);
  EXPECT_EQ(mem_access.is_store, true); // conditional store?
  EXPECT_EQ(mem_access.is_vector, false);
  EXPECT_EQ(mem_access.is_signed, false);
  EXPECT_EQ(mem_access.is_pair, true);
  EXPECT_EQ(mem_access.is_atomic, true);
  EXPECT_EQ(mem_access.size, 0b10);
  EXPECT_EQ(mem_access.accesses, 1); // conditional store?
}
