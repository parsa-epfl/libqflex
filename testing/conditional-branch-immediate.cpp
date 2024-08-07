#include <gtest/gtest.h>
#include "testing-utils.h"
#include "../libqflex/plugins/trace/trace.h"
#include "../libqflex/libqflex-legacy-api.h"
#include <bitset>

branch_type_t branch_type;

Field o1("x");
Field imm19("xxxxxxxxxxxxxxxxxxx");
Field o0("x");
Field cond("xxxx");

Instruction bitmask("0101010", o1, imm19, o0, cond);

TEST(ConditionalBranch_Immediate, B_Cond)
{
  o1 = "0";
  o0 = "0";

  for (uint32_t instr : bitmask) {
    ASSERT_EQ(decode_armv8_branch_opcode(&branch_type, instr), true);
    if (std::bitset<32>(instr).to_string().substr(28, 3) != "111") {
        ASSERT_EQ(branch_type, QEMU_Conditional_Branch) << "Instruction: " << std::bitset<32>(instr);
    } else {
        ASSERT_EQ(branch_type, QEMU_Unconditional_Branch) << "Instruction: " << std::bitset<32>(instr);
    }
  }
}

TEST(ConditionalBranch_Immediate, BC_Cond)
{
  o1 = "0";
  o0 = "1";

  for (uint32_t instr : bitmask) {
    ASSERT_EQ(decode_armv8_branch_opcode(&branch_type, instr), true);
    if (std::bitset<32>(instr).to_string().substr(28, 3) != "111") {
        ASSERT_EQ(branch_type, QEMU_Conditional_Branch) << "Instruction: " << std::bitset<32>(instr);
    } else {
        ASSERT_EQ(branch_type, QEMU_Unconditional_Branch) << "Instruction: " << std::bitset<32>(instr);
    }
  }
}
