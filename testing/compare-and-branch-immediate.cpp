#include <gtest/gtest.h>
#include "testing-utils.h"
#include "../libqflex/plugins/trace/trace.h"
#include "../libqflex/libqflex-legacy-api.h"

branch_type_t branch_type;

Field sf("x");
Field op("x");
Field imm19("xxxxxxxxxxxxxxxxxxx");
Field Rt("xxxxx");

Instruction bitmask(sf, "011010", op, imm19, Rt);

TEST(UnconditionalBranch_Immediate, CBZ_32Bit)
{
    sf = "0";
    op = "0";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_branch_opcode(&branch_type, instr), true);
        ASSERT_EQ(branch_type, QEMU_Conditional_Branch);
    }
}

TEST(UnconditionalBranch_Immediate, CBNZ_32Bit)
{
    sf = "0";
    op = "1";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_branch_opcode(&branch_type, instr), true);
        ASSERT_EQ(branch_type, QEMU_Conditional_Branch);
    }
}

TEST(UnconditionalBranch_Immediate, CBZ_64Bit)
{
    sf = "1";
    op = "0";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_branch_opcode(&branch_type, instr), true);
        ASSERT_EQ(branch_type, QEMU_Conditional_Branch);
    }
}

TEST(UnconditionalBranch_Immediate, CBNZ_64Bit)
{
    sf = "1";
    op = "1";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_branch_opcode(&branch_type, instr), true);
        ASSERT_EQ(branch_type, QEMU_Conditional_Branch);
    }
}
