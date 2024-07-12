#include <gtest/gtest.h>
#include "testing-utils.hh"

extern "C" {
  #include "../libqflex/plugins/trace/trace.h"
}

struct mem_access mem_access;

Field opc("xx");
Field VR("x"); /* is vector (simd&fp)? */
Field L("x"); /* is load? */
Field imm7("xxxxxxx");
Field Rt2("xxxxx");
Field Rn("xxxxx");
Field Rt("xxxxx");

Instruction bitmask(opc, "101", VR, "001", L, imm7, Rt2, Rn, Rt);


TEST(LoadStoreRegisterPair_PostIndexed, STP_32Bit)
{
  opc = "00";
  VR = "0";
  L = "0";

  for (uint32_t instr : bitmask) {
    ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
    ASSERT_EQ(mem_access.is_load, false);
    ASSERT_EQ(mem_access.is_store, true);
    ASSERT_EQ(mem_access.is_vector, false);
    ASSERT_EQ(mem_access.is_signed, false);
    ASSERT_EQ(mem_access.is_pair, true);
    ASSERT_EQ(mem_access.is_atomic, false);
    ASSERT_EQ(mem_access.size, 0b10);
    ASSERT_EQ(mem_access.accesses, 2);
  }
}

TEST(LoadStoreRegisterPair_PostIndexed, LDP_32Bit)
{
  opc = "00";
  VR = "0";
  L = "1";

  for (uint32_t instr : bitmask) {
    ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
    ASSERT_EQ(mem_access.is_load, true);
    ASSERT_EQ(mem_access.is_store, false);
    ASSERT_EQ(mem_access.is_vector, false);
    ASSERT_EQ(mem_access.is_signed, false);
    ASSERT_EQ(mem_access.is_pair, true);
    ASSERT_EQ(mem_access.is_atomic, false);
    ASSERT_EQ(mem_access.size, 0b10);
    ASSERT_EQ(mem_access.accesses, 2);
  }
}

TEST(LoadStoreRegisterPair_PostIndexed, STP_SIMDFP_32Bit)
{
  opc = "00";
  VR = "1";
  L = "0";

  for (uint32_t instr : bitmask) {
    ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
    ASSERT_EQ(mem_access.is_load, false);
    ASSERT_EQ(mem_access.is_store, true);
    ASSERT_EQ(mem_access.is_vector, true);
    ASSERT_EQ(mem_access.is_signed, false);
    ASSERT_EQ(mem_access.is_pair, true);
    ASSERT_EQ(mem_access.is_atomic, false);
    ASSERT_EQ(mem_access.size, 0b10);
    ASSERT_EQ(mem_access.accesses, 2); /* vector store? */
  }
}

TEST(LoadStoreRegisterPair_PostIndexed, LDP_SIMDFP_32Bit)
{
  opc = "00";
  VR = "1";
  L = "1";

  for (uint32_t instr : bitmask) {
    ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
    ASSERT_EQ(mem_access.is_load, true);
    ASSERT_EQ(mem_access.is_store, false);
    ASSERT_EQ(mem_access.is_vector, true);
    ASSERT_EQ(mem_access.is_signed, false);
    ASSERT_EQ(mem_access.is_pair, true);
    ASSERT_EQ(mem_access.is_atomic, false);
    ASSERT_EQ(mem_access.size, 0b10);
    ASSERT_EQ(mem_access.accesses, 2); /* vector load? */
  }
}

TEST(LoadStoreRegisterPair_PostIndexed, STGP)
{
  opc = "01";
  VR = "0";
  L = "0";

  for (uint32_t instr : bitmask) {
    ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
    ASSERT_EQ(mem_access.is_load, false);
    ASSERT_EQ(mem_access.is_store, true);
    ASSERT_EQ(mem_access.is_vector, false);
    ASSERT_EQ(mem_access.is_signed, false);
    ASSERT_EQ(mem_access.is_pair, true);
    ASSERT_EQ(mem_access.is_atomic, false);
    ASSERT_EQ(mem_access.size, 0b11);
    ASSERT_EQ(mem_access.accesses, 2); /* tag store? */
  }
}

TEST(LoadStoreRegisterPair_PostIndexed, LDPSW)
{
  opc = "01";
  VR = "0";
  L = "1";

  for (uint32_t instr : bitmask) {
    ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
    ASSERT_EQ(mem_access.is_load, true);
    ASSERT_EQ(mem_access.is_store, false);
    ASSERT_EQ(mem_access.is_vector, false);
    ASSERT_EQ(mem_access.is_signed, true);
    ASSERT_EQ(mem_access.is_pair, true);
    ASSERT_EQ(mem_access.is_atomic, false);
    ASSERT_EQ(mem_access.size, 0b10);
    ASSERT_EQ(mem_access.accesses, 2); /* tag load? */
  }
}

TEST(LoadStoreRegisterPair_PostIndexed, STP_SIMDFP_64Bit)
{
  opc = "01";
  VR = "1";
  L = "0";

  for (uint32_t instr : bitmask) {
    ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
    ASSERT_EQ(mem_access.is_load, false);
    ASSERT_EQ(mem_access.is_store, true);
    ASSERT_EQ(mem_access.is_vector, true);
    ASSERT_EQ(mem_access.is_signed, false);
    ASSERT_EQ(mem_access.is_pair, true);
    ASSERT_EQ(mem_access.is_atomic, false);
    ASSERT_EQ(mem_access.size, 0b11);
    ASSERT_EQ(mem_access.accesses, 2); /* vector store? */
  }
}

TEST(LoadStoreRegisterPair_PostIndexed, LDP_SIMDFP_64Bit)
{
  opc = "01";
  VR = "1";
  L = "1";

  for (uint32_t instr : bitmask) {
    ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
    ASSERT_EQ(mem_access.is_load, true);
    ASSERT_EQ(mem_access.is_store, false);
    ASSERT_EQ(mem_access.is_vector, true);
    ASSERT_EQ(mem_access.is_signed, false);
    ASSERT_EQ(mem_access.is_pair, true);
    ASSERT_EQ(mem_access.is_atomic, false);
    ASSERT_EQ(mem_access.size, 0b11);
    ASSERT_EQ(mem_access.accesses, 2); /* vector load? */
  }
}

TEST(LoadStoreRegisterPair_PostIndexed, STP_64Bit)
{
  opc = "10";
  VR = "0";
  L = "0";

  for (uint32_t instr : bitmask) {
    ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
    ASSERT_EQ(mem_access.is_load, false);
    ASSERT_EQ(mem_access.is_store, true);
    ASSERT_EQ(mem_access.is_vector, false);
    ASSERT_EQ(mem_access.is_signed, false);
    ASSERT_EQ(mem_access.is_pair, true);
    ASSERT_EQ(mem_access.is_atomic, false);
    ASSERT_EQ(mem_access.size, 0b11);
    ASSERT_EQ(mem_access.accesses, 2);
  }
}

TEST(LoadStoreRegisterPair_PostIndexed, LDP_64Bit)
{
  opc = "10";
  VR = "0";
  L = "1";

  for (uint32_t instr : bitmask) {
    ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
    ASSERT_EQ(mem_access.is_load, true);
    ASSERT_EQ(mem_access.is_store, false);
    ASSERT_EQ(mem_access.is_vector, false);
    ASSERT_EQ(mem_access.is_signed, false);
    ASSERT_EQ(mem_access.is_pair, true);
    ASSERT_EQ(mem_access.is_atomic, false);
    ASSERT_EQ(mem_access.size, 0b11);
    ASSERT_EQ(mem_access.accesses, 2);
  }
}

TEST(LoadStoreRegisterPair_PostIndexed, STP_SIMDFP_128Bit)
{
  opc = "10";
  VR = "1";
  L = "0";

  for (uint32_t instr : bitmask) {
    ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
    ASSERT_EQ(mem_access.is_load, false);
    ASSERT_EQ(mem_access.is_store, true);
    ASSERT_EQ(mem_access.is_vector, true);
    ASSERT_EQ(mem_access.is_signed, false);
    ASSERT_EQ(mem_access.is_pair, true);
    ASSERT_EQ(mem_access.is_atomic, false);
    ASSERT_EQ(mem_access.size, 0b100);
    ASSERT_EQ(mem_access.accesses, 2); /* vector store? */
  }
}

TEST(LoadStoreRegisterPair_PostIndexed, LDP_SIMDFP_128Bit)
{
  opc = "10";
  VR = "1";
  L = "1";

  for (uint32_t instr : bitmask) {
    ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
    ASSERT_EQ(mem_access.is_load, true);
    ASSERT_EQ(mem_access.is_store, false);
    ASSERT_EQ(mem_access.is_vector, true);
    ASSERT_EQ(mem_access.is_signed, false);
    ASSERT_EQ(mem_access.is_pair, true);
    ASSERT_EQ(mem_access.is_atomic, false);
    ASSERT_EQ(mem_access.size, 0b100);
    ASSERT_EQ(mem_access.accesses, 2); /* vector load? */
  }
}
