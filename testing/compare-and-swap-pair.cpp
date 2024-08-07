#include <gtest/gtest.h>
#include "testing-utils.h"
#include "../libqflex/plugins/trace/trace.h"

struct mem_access mem_access;

Field sz("x");
Field L("x");
Field Rs("xxxxx");
Field o0("x");
Field Rt2("11111");
Field Rn("xxxxx");
Field Rt("xxxxx");

Instruction bitmask("0", sz, "0010000", L, "1", Rs, o0, Rt2, Rn, Rt);

TEST(CompareAndSwapPair, CASP_32Bit)
{
  sz = "0";
  L = "0";
  o0 = "0";

  for (uint32_t instr : bitmask) {
    ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
    ASSERT_EQ(mem_access.is_load, true);
    ASSERT_EQ(mem_access.is_store, true); // conditional store?
    ASSERT_EQ(mem_access.is_vector, false);
    ASSERT_EQ(mem_access.is_signed, false);
    ASSERT_EQ(mem_access.is_pair, true);
    ASSERT_EQ(mem_access.is_atomic, true);
    ASSERT_EQ(mem_access.size, 0b10);
    ASSERT_EQ(mem_access.accesses, 4); // conditional store?
  }
}

TEST(CompareAndSwapPair, CASPL_32Bit)
{
  sz = "0";
  L = "0";
  o0 = "1";

  for (uint32_t instr : bitmask) {
    ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
    ASSERT_EQ(mem_access.is_load, true);
    ASSERT_EQ(mem_access.is_store, true); // conditional store?
    ASSERT_EQ(mem_access.is_vector, false);
    ASSERT_EQ(mem_access.is_signed, false);
    ASSERT_EQ(mem_access.is_pair, true);
    ASSERT_EQ(mem_access.is_atomic, true);
    ASSERT_EQ(mem_access.size, 0b10);
    ASSERT_EQ(mem_access.accesses, 4); // conditional store?
  }
}

TEST(CompareAndSwapPair, CASPA_32Bit)
{
  sz = "0";
  L = "1";
  o0 = "0";

  for (uint32_t instr : bitmask) {
    ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
    ASSERT_EQ(mem_access.is_load, true);
    ASSERT_EQ(mem_access.is_store, true); // conditional store?
    ASSERT_EQ(mem_access.is_vector, false);
    ASSERT_EQ(mem_access.is_signed, false);
    ASSERT_EQ(mem_access.is_pair, true);
    ASSERT_EQ(mem_access.is_atomic, true);
    ASSERT_EQ(mem_access.size, 0b10);
    ASSERT_EQ(mem_access.accesses, 4); // conditional store?
  }
}

TEST(CompareAndSwapPair, CASPAL_32Bit)
{
  sz = "0";
  L = "1";
  o0 = "1";

  for (uint32_t instr : bitmask) {
    ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
    ASSERT_EQ(mem_access.is_load, true);
    ASSERT_EQ(mem_access.is_store, true); // conditional store?
    ASSERT_EQ(mem_access.is_vector, false);
    ASSERT_EQ(mem_access.is_signed, false);
    ASSERT_EQ(mem_access.is_pair, true);
    ASSERT_EQ(mem_access.is_atomic, true);
    ASSERT_EQ(mem_access.size, 0b10);
    ASSERT_EQ(mem_access.accesses, 4); // conditional store?
  }
}

TEST(CompareAndSwapPair, CASP_64Bit)
{
  sz = "1";
  L = "0";
  o0 = "0";

  for (uint32_t instr : bitmask) {
    ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
    ASSERT_EQ(mem_access.is_load, true);
    ASSERT_EQ(mem_access.is_store, true); // conditional store?
    ASSERT_EQ(mem_access.is_vector, false);
    ASSERT_EQ(mem_access.is_signed, false);
    ASSERT_EQ(mem_access.is_pair, true);
    ASSERT_EQ(mem_access.is_atomic, true);
    ASSERT_EQ(mem_access.size, 0b11);
    ASSERT_EQ(mem_access.accesses, 4); // conditional store?
  }
}

TEST(CompareAndSwapPair, CASPL_64Bit)
{
  sz = "1";
  L = "0";
  o0 = "1";

  for (uint32_t instr : bitmask) {
    ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
    ASSERT_EQ(mem_access.is_load, true);
    ASSERT_EQ(mem_access.is_store, true); // conditional store?
    ASSERT_EQ(mem_access.is_vector, false);
    ASSERT_EQ(mem_access.is_signed, false);
    ASSERT_EQ(mem_access.is_pair, true);
    ASSERT_EQ(mem_access.is_atomic, true);
    ASSERT_EQ(mem_access.size, 0b11);
    ASSERT_EQ(mem_access.accesses, 4); // conditional store?
  }
}

TEST(CompareAndSwapPair, CASPA_64Bit)
{
  sz = "1";
  L = "1";
  o0 = "0";

  for (uint32_t instr : bitmask) {
    ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
    ASSERT_EQ(mem_access.is_load, true);
    ASSERT_EQ(mem_access.is_store, true); // conditional store?
    ASSERT_EQ(mem_access.is_vector, false);
    ASSERT_EQ(mem_access.is_signed, false);
    ASSERT_EQ(mem_access.is_pair, true);
    ASSERT_EQ(mem_access.is_atomic, true);
    ASSERT_EQ(mem_access.size, 0b11);
    ASSERT_EQ(mem_access.accesses, 4); // conditional store?
  }
}

TEST(CompareAndSwapPair, CASPAL_64Bit)
{
  sz = "1";
  L = "1";
  o0 = "1";

  for (uint32_t instr : bitmask) {
    ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
    ASSERT_EQ(mem_access.is_load, true);
    ASSERT_EQ(mem_access.is_store, true); // conditional store?
    ASSERT_EQ(mem_access.is_vector, false);
    ASSERT_EQ(mem_access.is_signed, false);
    ASSERT_EQ(mem_access.is_pair, true);
    ASSERT_EQ(mem_access.is_atomic, true);
    ASSERT_EQ(mem_access.size, 0b11);
    ASSERT_EQ(mem_access.accesses, 4); // conditional store?
  }
}
