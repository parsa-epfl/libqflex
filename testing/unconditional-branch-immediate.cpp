#include <gtest/gtest.h>
#include "testing-utils.h"
#include "../libqflex/plugins/trace/trace.h"
#include "../libqflex/libqflex-legacy-api.h"

branch_type_t branch_type;

Field op("x");
Field imm26("xxxxxxxxxxxxxxxxxxxxxxxxxx");

Instruction bitmask(op, "00101", imm26);

TEST(UnconditionalBranch_Immediate, B)
{
    op = "0";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_branch_opcode(&branch_type, instr), true);
        ASSERT_EQ(branch_type, QEMU_Unconditional_Branch);
    }
}

TEST(UnconditionalBranch_Immediate, BL)
{
    op = "1";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_branch_opcode(&branch_type, instr), true);
        ASSERT_EQ(branch_type, QEMU_Call_Branch);
    }
}
