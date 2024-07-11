#include <gtest/gtest.h>
#include "testing-utils.hh"

extern "C" {
  #include "../libqflex/plugins/trace/trace.h"
}

struct mem_access mem_access;

Field sz("x");
Field L("x");
Field Rs("xxxxx");
Field o0("x");
Field Rt2("11111");
Field Rn("xxxxx");
Field Rt("xxxxx");

Instruction bitmask("0", sz, "0010000", L, "1", Rs, o0, Rt2, Rn, Rt);

TEST(CompareAndSwapPair, CompareAndSwapPair32Bit)
{
  sz = "0";

  for (uint32_t instr : bitmask) {
    EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
    EXPECT_EQ(mem_access.is_load, true);
    EXPECT_EQ(mem_access.is_store, true); // conditional store?
    EXPECT_EQ(mem_access.is_vector, false);
    EXPECT_EQ(mem_access.is_signed, false);
    EXPECT_EQ(mem_access.is_pair, true);
    EXPECT_EQ(mem_access.is_atomic, true);
    EXPECT_EQ(mem_access.size, 0b10);
    EXPECT_EQ(mem_access.accesses, 4); // conditional store?
  }
}

TEST(CompareAndSwapPair, CompareAndSwapPair64Bit)
{
  sz = "1";

  for (uint32_t instr : bitmask) {
    EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
    EXPECT_EQ(mem_access.is_load, true);
    EXPECT_EQ(mem_access.is_store, true); // conditional store?
    EXPECT_EQ(mem_access.is_vector, false);
    EXPECT_EQ(mem_access.is_signed, false);
    EXPECT_EQ(mem_access.is_pair, true);
    EXPECT_EQ(mem_access.is_atomic, true);
    EXPECT_EQ(mem_access.size, 0b11);
    EXPECT_EQ(mem_access.accesses, 4); // conditional store?
  }
}
