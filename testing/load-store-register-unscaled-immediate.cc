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

Instruction bitmask(size, "111", VR, "00", opc, "0", imm9, "00", Rn, Rt);


TEST(LoadStoreRegister_UnscaledImmediate, STURB)
{
    size = "00";
    VR = "0";
    opc = "00";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        ASSERT_EQ(mem_access.is_load, false);
        ASSERT_EQ(mem_access.is_store, true);
        ASSERT_EQ(mem_access.is_vector, false);
        ASSERT_EQ(mem_access.is_signed, false);
        ASSERT_EQ(mem_access.is_pair, false);
        ASSERT_EQ(mem_access.is_atomic, false);
        ASSERT_EQ(mem_access.size, 0b00);
        ASSERT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_UnscaledImmediate, LDURB)
{
    size = "00";
    VR = "0";
    opc = "01";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        ASSERT_EQ(mem_access.is_load, true);
        ASSERT_EQ(mem_access.is_store, false);
        ASSERT_EQ(mem_access.is_vector, false);
        ASSERT_EQ(mem_access.is_signed, false);
        ASSERT_EQ(mem_access.is_pair, false);
        ASSERT_EQ(mem_access.is_atomic, false);
        ASSERT_EQ(mem_access.size, 0b00);
        ASSERT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_UnscaledImmediate, LDURSB_64Bit)
{
    size = "00";
    VR = "0";
    opc = "10";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        ASSERT_EQ(mem_access.is_load, true);
        ASSERT_EQ(mem_access.is_store, false);
        ASSERT_EQ(mem_access.is_vector, false);
        ASSERT_EQ(mem_access.is_signed, true);
        ASSERT_EQ(mem_access.is_pair, false);
        ASSERT_EQ(mem_access.is_atomic, false);
        ASSERT_EQ(mem_access.size, 0b00);
        ASSERT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_UnscaledImmediate, LDURSB_32Bit)
{
    size = "00";
    VR = "0";
    opc = "11";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        ASSERT_EQ(mem_access.is_load, true);
        ASSERT_EQ(mem_access.is_store, false);
        ASSERT_EQ(mem_access.is_vector, false);
        ASSERT_EQ(mem_access.is_signed, true);
        ASSERT_EQ(mem_access.is_pair, false);
        ASSERT_EQ(mem_access.is_atomic, false);
        ASSERT_EQ(mem_access.size, 0b00);
        ASSERT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_UnscaledImmediate, STUR_SIMDFP_8Bit)
{
    size = "00";
    VR = "1";
    opc = "00";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        ASSERT_EQ(mem_access.is_load, false);
        ASSERT_EQ(mem_access.is_store, true);
        ASSERT_EQ(mem_access.is_vector, true);
        ASSERT_EQ(mem_access.is_signed, false);
        ASSERT_EQ(mem_access.is_pair, false);
        ASSERT_EQ(mem_access.is_atomic, false);
        ASSERT_EQ(mem_access.size, 0b00);
        ASSERT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_UnscaledImmediate, LDUR_SIMDFP_8Bit)
{
    size = "00";
    VR = "1";
    opc = "01";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        ASSERT_EQ(mem_access.is_load, true);
        ASSERT_EQ(mem_access.is_store, false);
        ASSERT_EQ(mem_access.is_vector, true);
        ASSERT_EQ(mem_access.is_signed, false);
        ASSERT_EQ(mem_access.is_pair, false);
        ASSERT_EQ(mem_access.is_atomic, false);
        ASSERT_EQ(mem_access.size, 0b00);
        ASSERT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_UnscaledImmediate, STUR_SIMDFP_128Bit)
{
    size = "00";
    VR = "1";
    opc = "10";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        ASSERT_EQ(mem_access.is_load, false);
        ASSERT_EQ(mem_access.is_store, true);
        ASSERT_EQ(mem_access.is_vector, true);
        ASSERT_EQ(mem_access.is_signed, false);
        ASSERT_EQ(mem_access.is_pair, false);
        ASSERT_EQ(mem_access.is_atomic, false);
        ASSERT_EQ(mem_access.size, 0b100);
        ASSERT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_UnscaledImmediate, LDUR_SIMDFP_128Bit)
{
    size = "00";
    VR = "1";
    opc = "11";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        ASSERT_EQ(mem_access.is_load, true);
        ASSERT_EQ(mem_access.is_store, false);
        ASSERT_EQ(mem_access.is_vector, true);
        ASSERT_EQ(mem_access.is_signed, false);
        ASSERT_EQ(mem_access.is_pair, false);
        ASSERT_EQ(mem_access.is_atomic, false);
        ASSERT_EQ(mem_access.size, 0b100);
        ASSERT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_UnscaledImmediate, STURH)
{
    size = "01";
    VR = "0";
    opc = "00";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        ASSERT_EQ(mem_access.is_load, false);
        ASSERT_EQ(mem_access.is_store, true);
        ASSERT_EQ(mem_access.is_vector, false);
        ASSERT_EQ(mem_access.is_signed, false);
        ASSERT_EQ(mem_access.is_pair, false);
        ASSERT_EQ(mem_access.is_atomic, false);
        ASSERT_EQ(mem_access.size, 0b01);
        ASSERT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_UnscaledImmediate, LDURH)
{
    size = "01";
    VR = "0";
    opc = "01";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        ASSERT_EQ(mem_access.is_load, true);
        ASSERT_EQ(mem_access.is_store, false);
        ASSERT_EQ(mem_access.is_vector, false);
        ASSERT_EQ(mem_access.is_signed, false);
        ASSERT_EQ(mem_access.is_pair, false);
        ASSERT_EQ(mem_access.is_atomic, false);
        ASSERT_EQ(mem_access.size, 0b01);
        ASSERT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_UnscaledImmediate, LDURSH_64Bit)
{
    size = "01";
    VR = "0";
    opc = "10";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        ASSERT_EQ(mem_access.is_load, true);
        ASSERT_EQ(mem_access.is_store, false);
        ASSERT_EQ(mem_access.is_vector, false);
        ASSERT_EQ(mem_access.is_signed, true);
        ASSERT_EQ(mem_access.is_pair, false);
        ASSERT_EQ(mem_access.is_atomic, false);
        ASSERT_EQ(mem_access.size, 0b01);
        ASSERT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_UnscaledImmediate, LDURSH_32Bit)
{
    size = "01";
    VR = "0";
    opc = "11";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        ASSERT_EQ(mem_access.is_load, true);
        ASSERT_EQ(mem_access.is_store, false);
        ASSERT_EQ(mem_access.is_vector, false);
        ASSERT_EQ(mem_access.is_signed, true);
        ASSERT_EQ(mem_access.is_pair, false);
        ASSERT_EQ(mem_access.is_atomic, false);
        ASSERT_EQ(mem_access.size, 0b01);
        ASSERT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_UnscaledImmediate, STUR_SIMDFP_16Bit)
{
    size = "01";
    VR = "1";
    opc = "00";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        ASSERT_EQ(mem_access.is_load, false);
        ASSERT_EQ(mem_access.is_store, true);
        ASSERT_EQ(mem_access.is_vector, true);
        ASSERT_EQ(mem_access.is_signed, false);
        ASSERT_EQ(mem_access.is_pair, false);
        ASSERT_EQ(mem_access.is_atomic, false);
        ASSERT_EQ(mem_access.size, 0b01);
        ASSERT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_UnscaledImmediate, LDUR_SIMDFP_16Bit)
{
    size = "01";
    VR = "1";
    opc = "01";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        ASSERT_EQ(mem_access.is_load, true);
        ASSERT_EQ(mem_access.is_store, false);
        ASSERT_EQ(mem_access.is_vector, true);
        ASSERT_EQ(mem_access.is_signed, false);
        ASSERT_EQ(mem_access.is_pair, false);
        ASSERT_EQ(mem_access.is_atomic, false);
        ASSERT_EQ(mem_access.size, 0b01);
        ASSERT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_UnscaledImmediate, STUR_32Bit)
{
    size = "10";
    VR = "0";
    opc = "00";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        ASSERT_EQ(mem_access.is_load, false);
        ASSERT_EQ(mem_access.is_store, true);
        ASSERT_EQ(mem_access.is_vector, false);
        ASSERT_EQ(mem_access.is_signed, false);
        ASSERT_EQ(mem_access.is_pair, false);
        ASSERT_EQ(mem_access.is_atomic, false);
        ASSERT_EQ(mem_access.size, 0b10);
        ASSERT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_UnscaledImmediate, LDUR_32Bit)
{
    size = "10";
    VR = "0";
    opc = "01";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        ASSERT_EQ(mem_access.is_load, true);
        ASSERT_EQ(mem_access.is_store, false);
        ASSERT_EQ(mem_access.is_vector, false);
        ASSERT_EQ(mem_access.is_signed, false);
        ASSERT_EQ(mem_access.is_pair, false);
        ASSERT_EQ(mem_access.is_atomic, false);
        ASSERT_EQ(mem_access.size, 0b10);
        ASSERT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_UnscaledImmediate, LDURSW)
{
    size = "10";
    VR = "0";
    opc = "10";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        ASSERT_EQ(mem_access.is_load, true);
        ASSERT_EQ(mem_access.is_store, false);
        ASSERT_EQ(mem_access.is_vector, false);
        ASSERT_EQ(mem_access.is_signed, true);
        ASSERT_EQ(mem_access.is_pair, false);
        ASSERT_EQ(mem_access.is_atomic, false);
        ASSERT_EQ(mem_access.size, 0b10);
        ASSERT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_UnscaledImmediate, STUR_SIMDFP_32Bit)
{
    size = "10";
    VR = "1";
    opc = "00";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        ASSERT_EQ(mem_access.is_load, false);
        ASSERT_EQ(mem_access.is_store, true);
        ASSERT_EQ(mem_access.is_vector, true);
        ASSERT_EQ(mem_access.is_signed, false);
        ASSERT_EQ(mem_access.is_pair, false);
        ASSERT_EQ(mem_access.is_atomic, false);
        ASSERT_EQ(mem_access.size, 0b10);
        ASSERT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_UnscaledImmediate, LDUR_SIMDFP_32Bit)
{
    size = "10";
    VR = "1";
    opc = "01";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        ASSERT_EQ(mem_access.is_load, true);
        ASSERT_EQ(mem_access.is_store, false);
        ASSERT_EQ(mem_access.is_vector, true);
        ASSERT_EQ(mem_access.is_signed, false);
        ASSERT_EQ(mem_access.is_pair, false);
        ASSERT_EQ(mem_access.is_atomic, false);
        ASSERT_EQ(mem_access.size, 0b10);
        ASSERT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_UnscaledImmediate, STUR_64Bit)
{
    size = "11";
    VR = "0";
    opc = "00";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        ASSERT_EQ(mem_access.is_load, false);
        ASSERT_EQ(mem_access.is_store, true);
        ASSERT_EQ(mem_access.is_vector, false);
        ASSERT_EQ(mem_access.is_signed, false);
        ASSERT_EQ(mem_access.is_pair, false);
        ASSERT_EQ(mem_access.is_atomic, false);
        ASSERT_EQ(mem_access.size, 0b11);
        ASSERT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_UnscaledImmediate, LDUR_64Bit)
{
    size = "11";
    VR = "0";
    opc = "01";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        ASSERT_EQ(mem_access.is_load, true);
        ASSERT_EQ(mem_access.is_store, false);
        ASSERT_EQ(mem_access.is_vector, false);
        ASSERT_EQ(mem_access.is_signed, false);
        ASSERT_EQ(mem_access.is_pair, false);
        ASSERT_EQ(mem_access.is_atomic, false);
        ASSERT_EQ(mem_access.size, 0b11);
        ASSERT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_UnscaledImmediate, PRFUM)
{
    size = "11";
    VR = "0";
    opc = "10";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true); /* prefetch memory? */
        ASSERT_EQ(mem_access.is_load, false);
        ASSERT_EQ(mem_access.is_store, false);
        ASSERT_EQ(mem_access.is_vector, false);
        ASSERT_EQ(mem_access.is_signed, false);
        ASSERT_EQ(mem_access.is_pair, false);
        ASSERT_EQ(mem_access.is_atomic, false);
        ASSERT_EQ(mem_access.size, 0b11); /* prefetch memory? */
        ASSERT_EQ(mem_access.accesses, 1); /* prefetch memory? */
    }
}

TEST(LoadStoreRegister_UnscaledImmediate, STUR_SIMDFP_64Bit)
{
    size = "11";
    VR = "1";
    opc = "00";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        ASSERT_EQ(mem_access.is_load, false);
        ASSERT_EQ(mem_access.is_store, true);
        ASSERT_EQ(mem_access.is_vector, true);
        ASSERT_EQ(mem_access.is_signed, false);
        ASSERT_EQ(mem_access.is_pair, false);
        ASSERT_EQ(mem_access.is_atomic, false);
        ASSERT_EQ(mem_access.size, 0b11);
        ASSERT_EQ(mem_access.accesses, 1);
    }
}

TEST(LoadStoreRegister_UnscaledImmediate, LDUR_SIMDFP_64Bit)
{
    size = "11";
    VR = "1";
    opc = "01";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_mem_opcode(&mem_access, instr), true);
        ASSERT_EQ(mem_access.is_load, true);
        ASSERT_EQ(mem_access.is_store, false);
        ASSERT_EQ(mem_access.is_vector, true);
        ASSERT_EQ(mem_access.is_signed, false);
        ASSERT_EQ(mem_access.is_pair, false);
        ASSERT_EQ(mem_access.is_atomic, false);
        ASSERT_EQ(mem_access.size, 0b11);
        ASSERT_EQ(mem_access.accesses, 1);
    }
}
