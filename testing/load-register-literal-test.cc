#include <gtest/gtest.h>
#include "testing-utils.hh"

extern "C" {
  #include "../libqflex/plugins/trace/trace.h"
}

struct mem_access mem_access;

Field opc("xx");
Field vr("x");
Field imm19("xxxxxxxxxxxxxxxxxxx");
Field Rt("xxxxx");

Instruction bitmask(opc, "011", vr, "00", imm19, Rt);

TEST(Load_Register_Literal, LDR_Literal_32Bit)
{
  opc = "00";
  vr = "0";

  for (uint32_t instr : bitmask) {
    EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
    EXPECT_EQ(mem_access.is_load, true);
    EXPECT_EQ(mem_access.is_store, false);
    EXPECT_EQ(mem_access.is_vector, false);
    EXPECT_EQ(mem_access.is_pair, false);
    EXPECT_EQ(mem_access.is_atomic, false);
    EXPECT_EQ(mem_access.size, 0b10);
    EXPECT_EQ(mem_access.accesses, 1);
  }
}

TEST(Load_Register_Literal, LDR_Literal_SIMD_FP_32Bit)
{
  opc = "00";
  vr = "1";

  for (uint32_t instr : bitmask) {
    EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
    EXPECT_EQ(mem_access.is_load, true);
    EXPECT_EQ(mem_access.is_store, false);
    EXPECT_EQ(mem_access.is_vector, true);
    EXPECT_EQ(mem_access.is_pair, false);
    EXPECT_EQ(mem_access.is_atomic, false);
    EXPECT_EQ(mem_access.size, 0b10);
    EXPECT_EQ(mem_access.accesses, 1);
  }
}

TEST(Load_Register_Literal, LDR_Literal_64Bit)
{
  opc = "01";
  vr = "0";

  for (uint32_t instr : bitmask) {
    EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
    EXPECT_EQ(mem_access.is_load, true);
    EXPECT_EQ(mem_access.is_store, false);
    EXPECT_EQ(mem_access.is_vector, false);
    EXPECT_EQ(mem_access.is_pair, false);
    EXPECT_EQ(mem_access.is_atomic, false);
    EXPECT_EQ(mem_access.size, 0b11);
    EXPECT_EQ(mem_access.accesses, 1);
  }
}

TEST(Load_Register_Literal, LDR_Literal_SIMD_FP_64Bit)
{
  opc = "01";
  vr = "1";

  for (uint32_t instr : bitmask) {
    EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
    EXPECT_EQ(mem_access.is_load, true);
    EXPECT_EQ(mem_access.is_store, false);
    EXPECT_EQ(mem_access.is_vector, true);
    EXPECT_EQ(mem_access.is_pair, false);
    EXPECT_EQ(mem_access.is_atomic, false);
    EXPECT_EQ(mem_access.size, 0b11);
    EXPECT_EQ(mem_access.accesses, 1);
  }
}

TEST(Load_Register_Literal, LDRSW_Literal)
{
  opc = "10";
  vr = "0";

  for (uint32_t instr : bitmask) {
    EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
    EXPECT_EQ(mem_access.is_load, true);
    EXPECT_EQ(mem_access.is_store, false);
    EXPECT_EQ(mem_access.is_vector, false);
    EXPECT_EQ(mem_access.is_pair, false);
    EXPECT_EQ(mem_access.is_atomic, false);
    EXPECT_EQ(mem_access.size, 0b10);
    EXPECT_EQ(mem_access.accesses, 1);
  }
}

TEST(Load_Register_Literal, LDR_Literal_SIMD_FP_128Bit)
{
  opc = "10";
  vr = "1";

  for (uint32_t instr : bitmask) {
    EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
    EXPECT_EQ(mem_access.is_load, true);
    EXPECT_EQ(mem_access.is_store, false);
    EXPECT_EQ(mem_access.is_vector, true);
    EXPECT_EQ(mem_access.is_pair, false);
    EXPECT_EQ(mem_access.is_atomic, false);
    EXPECT_EQ(mem_access.size, 0b100);
    EXPECT_EQ(mem_access.accesses, 1);
  }
}
