#include <gtest/gtest.h>
#include "testing-utils.hh"

extern "C" {
  #include "../libqflex/plugins/trace/trace.h"
}

struct mem_access mem_access;

Field opc("xx");
Field VR("x"); /* is vector? */
Field _25to23("xxx"); /* addressing mode */
Field L("x"); /* is load? */
Field imm7("xxxxxxx");
Field Rt2("xxxxx");
Field Rn("xxxxx");
Field Rt("xxxxx");

Instruction bitmask(opc, "101", VR, _25to23, L, imm7, Rt2, Rn, Rt);

std::string _25to23_values[] = {"001", "010", "011"}; /* post-indexed, offset, pre-indexed */

TEST(Load_Store_Register_Pair, STP_32Bit)
{
  opc = "00";
  VR = "0";
  L = "0";

  for (std::string value : _25to23_values) {
    _25to23 = value;
    for (uint32_t instr : bitmask) {
      EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
      EXPECT_EQ(mem_access.is_load, false);
      EXPECT_EQ(mem_access.is_store, true);
      EXPECT_EQ(mem_access.is_vector, false);
      EXPECT_EQ(mem_access.is_signed, false);
      EXPECT_EQ(mem_access.is_pair, true);
      EXPECT_EQ(mem_access.is_atomic, false);
      EXPECT_EQ(mem_access.size, 0b10);
      EXPECT_EQ(mem_access.accesses, 2);
    }
  }
}

TEST(Load_Store_Register_Pair, LDP_32Bit)
{
  opc = "00";
  VR = "0";
  L = "1";

  for (std::string value : _25to23_values) {
    _25to23 = value;
    for (uint32_t instr : bitmask) {
      EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
      EXPECT_EQ(mem_access.is_load, true);
      EXPECT_EQ(mem_access.is_store, false);
      EXPECT_EQ(mem_access.is_vector, false);
      EXPECT_EQ(mem_access.is_signed, false);
      EXPECT_EQ(mem_access.is_pair, true);
      EXPECT_EQ(mem_access.is_atomic, false);
      EXPECT_EQ(mem_access.size, 0b10);
      EXPECT_EQ(mem_access.accesses, 2);
    }
  }
}

TEST(Load_Store_Register_Pair, STP_SIMD_FP_32Bit)
{
  opc = "00";
  VR = "1";
  L = "0";

  for (std::string value : _25to23_values) {
    _25to23 = value;
    for (uint32_t instr : bitmask) {
      EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
      EXPECT_EQ(mem_access.is_load, false);
      EXPECT_EQ(mem_access.is_store, true);
      EXPECT_EQ(mem_access.is_vector, true);
      EXPECT_EQ(mem_access.is_signed, false);
      EXPECT_EQ(mem_access.is_pair, true);
      EXPECT_EQ(mem_access.is_atomic, false);
      EXPECT_EQ(mem_access.size, 0b10);
      EXPECT_EQ(mem_access.accesses, 2); /* vector store? */
    }
  }
}

TEST(Load_Store_Register_Pair, LDP_SIMD_FP_32Bit)
{
  opc = "00";
  VR = "1";
  L = "1";

  for (std::string value : _25to23_values) {
    _25to23 = value;
    for (uint32_t instr : bitmask) {
      EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
      EXPECT_EQ(mem_access.is_load, true);
      EXPECT_EQ(mem_access.is_store, false);
      EXPECT_EQ(mem_access.is_vector, true);
      EXPECT_EQ(mem_access.is_signed, false);
      EXPECT_EQ(mem_access.is_pair, true);
      EXPECT_EQ(mem_access.is_atomic, false);
      EXPECT_EQ(mem_access.size, 0b10);
      EXPECT_EQ(mem_access.accesses, 2); /* vector load? */
    }
  }
}

TEST(Load_Store_Register_Pair, STGP)
{
  opc = "01";
  VR = "0";
  L = "0";

  for (std::string value : _25to23_values) {
    _25to23 = value;
    for (uint32_t instr : bitmask) {
      EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
      EXPECT_EQ(mem_access.is_load, false);
      EXPECT_EQ(mem_access.is_store, true);
      EXPECT_EQ(mem_access.is_vector, false);
      EXPECT_EQ(mem_access.is_signed, false);
      EXPECT_EQ(mem_access.is_pair, true);
      EXPECT_EQ(mem_access.is_atomic, false);
      EXPECT_EQ(mem_access.size, 0b11);
      EXPECT_EQ(mem_access.accesses, 2); /* tag store? */
    }
  }
}

TEST(Load_Store_Register_Pair, LDPSW)
{
  opc = "01";
  VR = "0";
  L = "1";

  for (std::string value : _25to23_values) {
    _25to23 = value;
    for (uint32_t instr : bitmask) {
      EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
      EXPECT_EQ(mem_access.is_load, true);
      EXPECT_EQ(mem_access.is_store, false);
      EXPECT_EQ(mem_access.is_vector, false);
      EXPECT_EQ(mem_access.is_signed, true);
      EXPECT_EQ(mem_access.is_pair, true);
      EXPECT_EQ(mem_access.is_atomic, false);
      EXPECT_EQ(mem_access.size, 0b11);
      EXPECT_EQ(mem_access.accesses, 2); /* tag load? */
    }
  }
}

TEST(Load_Store_Register_Pair, STP_SIMD_FP_64Bit)
{
  opc = "01";
  VR = "1";
  L = "0";

  for (std::string value : _25to23_values) {
    _25to23 = value;
    for (uint32_t instr : bitmask) {
      EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
      EXPECT_EQ(mem_access.is_load, false);
      EXPECT_EQ(mem_access.is_store, true);
      EXPECT_EQ(mem_access.is_vector, true);
      EXPECT_EQ(mem_access.is_signed, false);
      EXPECT_EQ(mem_access.is_pair, true);
      EXPECT_EQ(mem_access.is_atomic, false);
      EXPECT_EQ(mem_access.size, 0b11);
      EXPECT_EQ(mem_access.accesses, 2); /* vector store? */
    }
  }
}

TEST(Load_Store_Register_Pair, LDP_SIMD_FP_64Bit)
{
  opc = "01";
  VR = "1";
  L = "1";

  for (std::string value : _25to23_values) {
    _25to23 = value;
    for (uint32_t instr : bitmask) {
      EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
      EXPECT_EQ(mem_access.is_load, true);
      EXPECT_EQ(mem_access.is_store, false);
      EXPECT_EQ(mem_access.is_vector, true);
      EXPECT_EQ(mem_access.is_signed, false);
      EXPECT_EQ(mem_access.is_pair, true);
      EXPECT_EQ(mem_access.is_atomic, false);
      EXPECT_EQ(mem_access.size, 0b11);
      EXPECT_EQ(mem_access.accesses, 2); /* vector load? */
    }
  }
}

TEST(Load_Store_Register_Pair, STP_64Bit)
{
  opc = "10";
  VR = "0";
  L = "0";

  for (std::string value : _25to23_values) {
    _25to23 = value;
    for (uint32_t instr : bitmask) {
      EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
      EXPECT_EQ(mem_access.is_load, false);
      EXPECT_EQ(mem_access.is_store, true);
      EXPECT_EQ(mem_access.is_vector, false);
      EXPECT_EQ(mem_access.is_signed, false);
      EXPECT_EQ(mem_access.is_pair, true);
      EXPECT_EQ(mem_access.is_atomic, false);
      EXPECT_EQ(mem_access.size, 0b11);
      EXPECT_EQ(mem_access.accesses, 2);
    }
  }
}

TEST(Load_Store_Register_Pair, LDP_64Bit)
{
  opc = "10";
  VR = "0";
  L = "1";

  for (std::string value : _25to23_values) {
    _25to23 = value;
    for (uint32_t instr : bitmask) {
      EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
      EXPECT_EQ(mem_access.is_load, true);
      EXPECT_EQ(mem_access.is_store, false);
      EXPECT_EQ(mem_access.is_vector, false);
      EXPECT_EQ(mem_access.is_signed, false);
      EXPECT_EQ(mem_access.is_pair, true);
      EXPECT_EQ(mem_access.is_atomic, false);
      EXPECT_EQ(mem_access.size, 0b11);
      EXPECT_EQ(mem_access.accesses, 2);
    }
  }
}

TEST(Load_Store_Register_Pair, STP_SIMD_FP_128Bit)
{
  opc = "10";
  VR = "1";
  L = "0";

  for (std::string value : _25to23_values) {
    _25to23 = value;
    for (uint32_t instr : bitmask) {
      EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
      EXPECT_EQ(mem_access.is_load, false);
      EXPECT_EQ(mem_access.is_store, true);
      EXPECT_EQ(mem_access.is_vector, true);
      EXPECT_EQ(mem_access.is_signed, false);
      EXPECT_EQ(mem_access.is_pair, true);
      EXPECT_EQ(mem_access.is_atomic, false);
      EXPECT_EQ(mem_access.size, 0b11);
      EXPECT_EQ(mem_access.accesses, 2); /* vector store? */
    }
  }
}

TEST(Load_Store_Register_Pair, LDP_SIMD_FP_128Bit)
{
  opc = "10";
  VR = "1";
  L = "1";

  for (std::string value : _25to23_values) {
    _25to23 = value;
    for (uint32_t instr : bitmask) {
      EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
      EXPECT_EQ(mem_access.is_load, true);
      EXPECT_EQ(mem_access.is_store, false);
      EXPECT_EQ(mem_access.is_vector, true);
      EXPECT_EQ(mem_access.is_signed, false);
      EXPECT_EQ(mem_access.is_pair, true);
      EXPECT_EQ(mem_access.is_atomic, false);
      EXPECT_EQ(mem_access.size, 0b11);
      EXPECT_EQ(mem_access.accesses, 2); /* vector load? */
    }
  }
}
