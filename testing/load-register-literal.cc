#include <gtest/gtest.h>
#include "testing-utils.hh"

extern "C" {
  #include "../libqflex/plugins/trace/trace.h"
}

struct mem_access mem_access;

Field opc("xx");
Field VR("x");
Field imm19("xxxxxxxxxxxxxxxxxxx");
Field Rt("xxxxx");

Instruction bitmask(opc, "011", VR, "00", imm19, Rt);

TEST(LoadRegister_Literal, LDR_32Bit)
{
  opc = "00";
  VR = "0";

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

TEST(LoadRegister_Literal, LDR_SIMDFP_32Bit)
{
  opc = "00";
  VR = "1";

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

TEST(LoadRegister_Literal, LDR_64Bit)
{
  opc = "01";
  VR = "0";

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

TEST(LoadRegister_Literal, LDR_SIMDFP_64Bit)
{
  opc = "01";
  VR = "1";

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

TEST(LoadRegister_Literal, LDRSW)
{
  opc = "10";
  VR = "0";

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

TEST(LoadRegister_Literal, LDR_SIMDFP_128Bit)
{
  opc = "10";
  VR = "1";

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

TEST(LoadRegister_Literal, PRFM)
{
  opc = "11";
  VR = "0";

  for (uint32_t instr : bitmask) {
    EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
    EXPECT_EQ(mem_access.is_load, false);
    EXPECT_EQ(mem_access.is_store, false);
    EXPECT_EQ(mem_access.is_vector, false);
    EXPECT_EQ(mem_access.is_pair, false);
    EXPECT_EQ(mem_access.is_atomic, false);
    EXPECT_EQ(mem_access.size, 0b10); /* prefetch? */
    EXPECT_EQ(mem_access.accesses, 1); /* prefetch? */
  }
}
