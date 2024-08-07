#include <gtest/gtest.h>
#include "testing-utils.h"
#include "../libqflex/plugins/trace/trace.h"
#include "../libqflex/libqflex-legacy-api.h"

branch_type_t branch_type;

Field b5("x");
Field op("x");
Field b40("xxxxx");
Field imm14("xxxxxxxxxxxxxx");
Field Rt("xxxxx");

Instruction bitmask(b5, "011011", op, b40, imm14, Rt);

TEST(CompareAndBranch_Immediate, TBZ)
{
    op = "0";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_branch_opcode(&branch_type, instr), true);
        ASSERT_EQ(branch_type, QEMU_Conditional_Branch);
    }
}

TEST(CompareAndBranch_Immediate, TBNZ)
{
    op = "1";

    for (uint32_t instr : bitmask) {
        ASSERT_EQ(decode_armv8_branch_opcode(&branch_type, instr), true);
        ASSERT_EQ(branch_type, QEMU_Conditional_Branch);
    }
}
