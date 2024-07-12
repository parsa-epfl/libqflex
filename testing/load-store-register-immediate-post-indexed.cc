#include <gtest/gtest.h>
#include "testing-utils.hh"

extern "C" {
  #include "../libqflex/plugins/trace/trace.h"
}

struct mem_access mem_access;

Field size("xx");
Field VR("x"); /* is vector (simd&fp)? */
Field opc("xx");
Field imm9("xxxxxxxxx");
Field Rn("xxxxx");
Field Rt("xxxxx");

Instruction bitmask(size, "111", VR, "00", opc, "0", imm9, "01", Rn, Rt);

TEST(LoadStoreRegister_ImmediatePostIndexed, STRB)
{
    size = "00";
    VR = "0";
    opc = "00";

    for (uint32_t instr : bitmask) {
        EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        EXPECT_EQ(mem_access.is_load, false);
        EXPECT_EQ(mem_access.is_store, true);
        EXPECT_EQ(mem_access.is_vector, false);
        EXPECT_EQ(mem_access.is_signed, false);
        EXPECT_EQ(mem_access.is_pair, false);
        EXPECT_EQ(mem_access.is_atomic, false);
        EXPECT_EQ(mem_access.size, 0b00);
        EXPECT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_ImmediatePostIndexed, LDRB)
{
    size = "00";
    VR = "0";
    opc = "01";

    for (uint32_t instr : bitmask) {
        EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        EXPECT_EQ(mem_access.is_load, true);
        EXPECT_EQ(mem_access.is_store, false);
        EXPECT_EQ(mem_access.is_vector, false);
        EXPECT_EQ(mem_access.is_signed, false);
        EXPECT_EQ(mem_access.is_pair, false);
        EXPECT_EQ(mem_access.is_atomic, false);
        EXPECT_EQ(mem_access.size, 0b00);
        EXPECT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_ImmediatePostIndexed, LDRSB_64Bit)
{
    size = "00";
    VR = "0";
    opc = "10";

    for (uint32_t instr : bitmask) {
        EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        EXPECT_EQ(mem_access.is_load, true);
        EXPECT_EQ(mem_access.is_store, false);
        EXPECT_EQ(mem_access.is_vector, false);
        EXPECT_EQ(mem_access.is_signed, true);
        EXPECT_EQ(mem_access.is_pair, false);
        EXPECT_EQ(mem_access.is_atomic, false);
        EXPECT_EQ(mem_access.size, 0b00);
        EXPECT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_ImmediatePostIndexed, LDRSB_32Bit)
{
    size = "00";
    VR = "0";
    opc = "11";

    for (uint32_t instr : bitmask) {
        EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        EXPECT_EQ(mem_access.is_load, true);
        EXPECT_EQ(mem_access.is_store, false);
        EXPECT_EQ(mem_access.is_vector, false);
        EXPECT_EQ(mem_access.is_signed, true);
        EXPECT_EQ(mem_access.is_pair, false);
        EXPECT_EQ(mem_access.is_atomic, false);
        EXPECT_EQ(mem_access.size, 0b00);
        EXPECT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_ImmediatePostIndexed, STR_SIMDFP_8Bit)
{
    size = "00";
    VR = "1";
    opc = "00";

    for (uint32_t instr : bitmask) {
        EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        EXPECT_EQ(mem_access.is_load, false);
        EXPECT_EQ(mem_access.is_store, true);
        EXPECT_EQ(mem_access.is_vector, true);
        EXPECT_EQ(mem_access.is_signed, false);
        EXPECT_EQ(mem_access.is_pair, false);
        EXPECT_EQ(mem_access.is_atomic, false);
        EXPECT_EQ(mem_access.size, 0b00);
        EXPECT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_ImmediatePostIndexed, LDR_SIMDFP_8Bit)
{
    size = "00";
    VR = "1";
    opc = "01";

    for (uint32_t instr : bitmask) {
        EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        EXPECT_EQ(mem_access.is_load, true);
        EXPECT_EQ(mem_access.is_store, false);
        EXPECT_EQ(mem_access.is_vector, true);
        EXPECT_EQ(mem_access.is_signed, false);
        EXPECT_EQ(mem_access.is_pair, false);
        EXPECT_EQ(mem_access.is_atomic, false);
        EXPECT_EQ(mem_access.size, 0b00);
        EXPECT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_ImmediatePostIndexed, STR_SIMDFP_128Bit)
{
    size = "00";
    VR = "1";
    opc = "10";

    for (uint32_t instr : bitmask) {
        EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        EXPECT_EQ(mem_access.is_load, false);
        EXPECT_EQ(mem_access.is_store, true);
        EXPECT_EQ(mem_access.is_vector, true);
        EXPECT_EQ(mem_access.is_signed, false);
        EXPECT_EQ(mem_access.is_pair, false);
        EXPECT_EQ(mem_access.is_atomic, false);
        EXPECT_EQ(mem_access.size, 0b100);
        EXPECT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_ImmediatePostIndexed, LDR_SIMDFP_128Bit)
{
    size = "00";
    VR = "1";
    opc = "11";

    for (uint32_t instr : bitmask) {
        EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        EXPECT_EQ(mem_access.is_load, true);
        EXPECT_EQ(mem_access.is_store, false);
        EXPECT_EQ(mem_access.is_vector, true);
        EXPECT_EQ(mem_access.is_signed, false);
        EXPECT_EQ(mem_access.is_pair, false);
        EXPECT_EQ(mem_access.is_atomic, false);
        EXPECT_EQ(mem_access.size, 0b100);
        EXPECT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_ImmediatePostIndexed, STRH)
{
    size = "01";
    VR = "0";
    opc = "00";

    for (uint32_t instr : bitmask) {
        EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        EXPECT_EQ(mem_access.is_load, false);
        EXPECT_EQ(mem_access.is_store, true);
        EXPECT_EQ(mem_access.is_vector, false);
        EXPECT_EQ(mem_access.is_signed, false);
        EXPECT_EQ(mem_access.is_pair, false);
        EXPECT_EQ(mem_access.is_atomic, false);
        EXPECT_EQ(mem_access.size, 0b01);
        EXPECT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_ImmediatePostIndexed, LDRH)
{
    size = "01";
    VR = "0";
    opc = "01";

    for (uint32_t instr : bitmask) {
        EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        EXPECT_EQ(mem_access.is_load, true);
        EXPECT_EQ(mem_access.is_store, false);
        EXPECT_EQ(mem_access.is_vector, false);
        EXPECT_EQ(mem_access.is_signed, false);
        EXPECT_EQ(mem_access.is_pair, false);
        EXPECT_EQ(mem_access.is_atomic, false);
        EXPECT_EQ(mem_access.size, 0b01);
        EXPECT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_ImmediatePostIndexed, LDRSH_64Bit)
{
    size = "01";
    VR = "0";
    opc = "10";

    for (uint32_t instr : bitmask) {
        EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        EXPECT_EQ(mem_access.is_load, true);
        EXPECT_EQ(mem_access.is_store, false);
        EXPECT_EQ(mem_access.is_vector, false);
        EXPECT_EQ(mem_access.is_signed, true);
        EXPECT_EQ(mem_access.is_pair, false);
        EXPECT_EQ(mem_access.is_atomic, false);
        EXPECT_EQ(mem_access.size, 0b01);
        EXPECT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_ImmediatePostIndexed, LDRSH_32Bit)
{
    size = "01";
    VR = "0";
    opc = "11";

    for (uint32_t instr : bitmask) {
        EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        EXPECT_EQ(mem_access.is_load, true);
        EXPECT_EQ(mem_access.is_store, false);
        EXPECT_EQ(mem_access.is_vector, false);
        EXPECT_EQ(mem_access.is_signed, true);
        EXPECT_EQ(mem_access.is_pair, false);
        EXPECT_EQ(mem_access.is_atomic, false);
        EXPECT_EQ(mem_access.size, 0b01);
        EXPECT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_ImmediatePostIndexed, STR_SIMDFP_16Bit)
{
    size = "01";
    VR = "1";
    opc = "00";

    for (uint32_t instr : bitmask) {
        EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        EXPECT_EQ(mem_access.is_load, false);
        EXPECT_EQ(mem_access.is_store, true);
        EXPECT_EQ(mem_access.is_vector, true);
        EXPECT_EQ(mem_access.is_signed, false);
        EXPECT_EQ(mem_access.is_pair, false);
        EXPECT_EQ(mem_access.is_atomic, false);
        EXPECT_EQ(mem_access.size, 0b01);
        EXPECT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_ImmediatePostIndexed, LDR_SIMDFP_16Bit)
{
    size = "01";
    VR = "1";
    opc = "01";

    for (uint32_t instr : bitmask) {
        EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        EXPECT_EQ(mem_access.is_load, true);
        EXPECT_EQ(mem_access.is_store, false);
        EXPECT_EQ(mem_access.is_vector, true);
        EXPECT_EQ(mem_access.is_signed, false);
        EXPECT_EQ(mem_access.is_pair, false);
        EXPECT_EQ(mem_access.is_atomic, false);
        EXPECT_EQ(mem_access.size, 0b01);
        EXPECT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_ImmediatePostIndexed, STR_32Bit)
{
    size = "10";
    VR = "0";
    opc = "00";

    for (uint32_t instr : bitmask) {
        EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        EXPECT_EQ(mem_access.is_load, false);
        EXPECT_EQ(mem_access.is_store, true);
        EXPECT_EQ(mem_access.is_vector, false);
        EXPECT_EQ(mem_access.is_signed, false);
        EXPECT_EQ(mem_access.is_pair, false);
        EXPECT_EQ(mem_access.is_atomic, false);
        EXPECT_EQ(mem_access.size, 0b10);
        EXPECT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_ImmediatePostIndexed, LDR_32Bit)
{
    size = "10";
    VR = "0";
    opc = "01";

    for (uint32_t instr : bitmask) {
        EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        EXPECT_EQ(mem_access.is_load, true);
        EXPECT_EQ(mem_access.is_store, false);
        EXPECT_EQ(mem_access.is_vector, false);
        EXPECT_EQ(mem_access.is_signed, false);
        EXPECT_EQ(mem_access.is_pair, false);
        EXPECT_EQ(mem_access.is_atomic, false);
        EXPECT_EQ(mem_access.size, 0b10);
        EXPECT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_ImmediatePostIndexed, LDRSW)
{
    size = "10";
    VR = "0";
    opc = "10";

    for (uint32_t instr : bitmask) {
        EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        EXPECT_EQ(mem_access.is_load, true);
        EXPECT_EQ(mem_access.is_store, false);
        EXPECT_EQ(mem_access.is_vector, false);
        EXPECT_EQ(mem_access.is_signed, true);
        EXPECT_EQ(mem_access.is_pair, false);
        EXPECT_EQ(mem_access.is_atomic, false);
        EXPECT_EQ(mem_access.size, 0b10);
        EXPECT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_ImmediatePostIndexed, STR_SIMDFP_32Bit)
{
    size = "10";
    VR = "1";
    opc = "00";

    for (uint32_t instr : bitmask) {
        EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        EXPECT_EQ(mem_access.is_load, false);
        EXPECT_EQ(mem_access.is_store, true);
        EXPECT_EQ(mem_access.is_vector, true);
        EXPECT_EQ(mem_access.is_signed, false);
        EXPECT_EQ(mem_access.is_pair, false);
        EXPECT_EQ(mem_access.is_atomic, false);
        EXPECT_EQ(mem_access.size, 0b10);
        EXPECT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_ImmediatePostIndexed, LDR_SIMDFP_32Bit)
{
    size = "10";
    VR = "1";
    opc = "01";

    for (uint32_t instr : bitmask) {
        EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        EXPECT_EQ(mem_access.is_load, true);
        EXPECT_EQ(mem_access.is_store, false);
        EXPECT_EQ(mem_access.is_vector, true);
        EXPECT_EQ(mem_access.is_signed, false);
        EXPECT_EQ(mem_access.is_pair, false);
        EXPECT_EQ(mem_access.is_atomic, false);
        EXPECT_EQ(mem_access.size, 0b10);
        EXPECT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_ImmediatePostIndexed, STR_64Bit)
{
    size = "11";
    VR = "0";
    opc = "00";

    for (uint32_t instr : bitmask) {
        EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        EXPECT_EQ(mem_access.is_load, false);
        EXPECT_EQ(mem_access.is_store, true);
        EXPECT_EQ(mem_access.is_vector, false);
        EXPECT_EQ(mem_access.is_signed, false);
        EXPECT_EQ(mem_access.is_pair, false);
        EXPECT_EQ(mem_access.is_atomic, false);
        EXPECT_EQ(mem_access.size, 0b11);
        EXPECT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_ImmediatePostIndexed, LDR_64Bit)
{
    size = "11";
    VR = "0";
    opc = "01";

    for (uint32_t instr : bitmask) {
        EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        EXPECT_EQ(mem_access.is_load, true);
        EXPECT_EQ(mem_access.is_store, false);
        EXPECT_EQ(mem_access.is_vector, false);
        EXPECT_EQ(mem_access.is_signed, false);
        EXPECT_EQ(mem_access.is_pair, false);
        EXPECT_EQ(mem_access.is_atomic, false);
        EXPECT_EQ(mem_access.size, 0b11);
        EXPECT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_ImmediatePostIndexed, STR_SIMDFP_64Bit)
{
    size = "11";
    VR = "1";
    opc = "00";

    for (uint32_t instr : bitmask) {
        EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        EXPECT_EQ(mem_access.is_load, false);
        EXPECT_EQ(mem_access.is_store, true);
        EXPECT_EQ(mem_access.is_vector, true);
        EXPECT_EQ(mem_access.is_signed, false);
        EXPECT_EQ(mem_access.is_pair, false);
        EXPECT_EQ(mem_access.is_atomic, false);
        EXPECT_EQ(mem_access.size, 0b11);
        EXPECT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_ImmediatePostIndexed, LDR_SIMDFP_64Bit)
{
    size = "11";
    VR = "1";
    opc = "01";

    for (uint32_t instr : bitmask) {
        EXPECT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        EXPECT_EQ(mem_access.is_load, true);
        EXPECT_EQ(mem_access.is_store, false);
        EXPECT_EQ(mem_access.is_vector, true);
        EXPECT_EQ(mem_access.is_signed, false);
        EXPECT_EQ(mem_access.is_pair, false);
        EXPECT_EQ(mem_access.is_atomic, false);
        EXPECT_EQ(mem_access.size, 0b11);
        EXPECT_EQ(mem_access.accesses, 1);
    }
}
