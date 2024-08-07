#include <gtest/gtest.h>
#include "testing-utils.h"
#include "../libqflex/plugins/trace/trace.h"
#include "../libqflex/libqflex-legacy-api.h"

branch_type_t branch_type;

Field opc("xxxx");
Field op2("11111");
Field op3("xxxxxx");
Field Rn("xxxxx");
Field op4("xxxxx");

Instruction bitmask("1101011", opc, op2, op3, Rn, op4);

TEST(UnconditionalBranch_Register, BR)
{
    opc = "0000";
    op3 = "000000";
    op4 = "00000";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_branch_opcode(&branch_type, instr), true);
        ASSERT_EQ(branch_type, QEMU_IndirectReg_Branch);
    }
}

TEST(UnconditionalBranch_Register, BRAAZ)
{
    opc = "0000";
    op3 = "000010";
    op4 = "11111";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_branch_opcode(&branch_type, instr), true);
        ASSERT_EQ(branch_type, QEMU_IndirectReg_Branch);
    }
}

TEST(UnconditionalBranch_Register, BRABZ)
{
    opc = "0000";
    op3 = "000011";
    op4 = "11111";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_branch_opcode(&branch_type, instr), true);
        ASSERT_EQ(branch_type, QEMU_IndirectReg_Branch);
    }
}

TEST(UnconditionalBranch_Register, BLR)
{
    opc = "0001";
    op3 = "000000";
    op4 = "00000";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_branch_opcode(&branch_type, instr), true);
        ASSERT_EQ(branch_type, QEMU_IndirectCall_Branch);
    }
}

TEST(UnconditionalBranch_Register, BLRAAZ)
{
    opc = "0001";
    op3 = "000010";
    op4 = "11111";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_branch_opcode(&branch_type, instr), true);
        ASSERT_EQ(branch_type, QEMU_IndirectCall_Branch);
    }
}

TEST(UnconditionalBranch_Register, BLRABZ)
{
    opc = "0001";
    op3 = "000011";
    op4 = "11111";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_branch_opcode(&branch_type, instr), true);
        ASSERT_EQ(branch_type, QEMU_IndirectCall_Branch);
    }
}

TEST(UnconditionalBranch_Register, RET)
{
    opc = "0010";
    op3 = "000000";
    op4 = "00000";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_branch_opcode(&branch_type, instr), true);
        ASSERT_EQ(branch_type, QEMU_Return_Branch);
    }
}

TEST(UnconditionalBranch_Register, RETAA)
{
    opc = "0010";
    op3 = "000010";
    Rn = "11111";
    op4 = "11111";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_branch_opcode(&branch_type, instr), true);
        ASSERT_EQ(branch_type, QEMU_Return_Branch);
    }
}

TEST(UnconditionalBranch_Register, RETAB)
{
    opc = "0010";
    op3 = "000011";
    Rn = "11111";
    op4 = "11111";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_branch_opcode(&branch_type, instr), true);
        ASSERT_EQ(branch_type, QEMU_Return_Branch);
    }
}

TEST(UnconditionalBranch_Register, ERET)
{
    opc = "0100";
    op3 = "000000";
    Rn = "11111";
    op4 = "00000";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_branch_opcode(&branch_type, instr), true);
        ASSERT_EQ(branch_type, QEMU_Return_Branch);
    }
}

TEST(UnconditionalBranch_Register, ERETAA)
{
    opc = "0100";
    op3 = "000010";
    Rn = "11111";
    op4 = "11111";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_branch_opcode(&branch_type, instr), true);
        ASSERT_EQ(branch_type, QEMU_Return_Branch);
    }
}

TEST(UnconditionalBranch_Register, ERETAB)
{
    opc = "0100";
    op3 = "000011";
    Rn = "11111";
    op4 = "11111";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_branch_opcode(&branch_type, instr), true);
        ASSERT_EQ(branch_type, QEMU_Return_Branch);
    }
}

TEST(UnconditionalBranch_Register, DRPS)
{
    opc = "0101";
    op3 = "000000";
    Rn = "11111";
    op4 = "00000";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_branch_opcode(&branch_type, instr), false); // ?
    }
}

TEST(UnconditionalBranch_Register, BRAA)
{
    opc = "1000";
    op3 = "000010";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_branch_opcode(&branch_type, instr), true);
        ASSERT_EQ(branch_type, QEMU_IndirectReg_Branch);
    }
}

TEST(UnconditionalBranch_Register, BRAB)
{
    opc = "1000";
    op3 = "000011";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_branch_opcode(&branch_type, instr), true);
        ASSERT_EQ(branch_type, QEMU_IndirectReg_Branch);
    }
}

TEST(UnconditionalBranch_Register, BLRAA)
{
    opc = "1001";
    op3 = "000010";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_branch_opcode(&branch_type, instr), true);
        ASSERT_EQ(branch_type, QEMU_IndirectCall_Branch);
    }
}

TEST(UnconditionalBranch_Register, BLRAB)
{
    opc = "1001";
    op3 = "000011";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_branch_opcode(&branch_type, instr), true);
        ASSERT_EQ(branch_type, QEMU_IndirectCall_Branch);
    }
}
