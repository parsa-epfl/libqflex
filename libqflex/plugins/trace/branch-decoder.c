#include "qemu/osdep.h"
#include "qemu/bitops.h"

#include "trace.h"


/* Unconditional branch (immediate)
 *   31  30       26 25                                  0
 * +----+-----------+-------------------------------------+
 * | op | 0 0 1 0 1 |                 imm26               |
 * +----+-----------+-------------------------------------+
 */
static bool
disas_uncond_b_imm(branch_type_t* s, uint32_t opcode)
{
    *s = QEMU_Unconditional_Branch;
    if (opcode & (1U << 31)) {
        /* BL Branch with link */
        // gen_pc_plus_diff(s, cpu_reg(s, 30), curr_opcode_len(s));
        *s = QEMU_Call_Branch;
    }
    /* B Branch / BL Branch with link */
    // reset_btype(s);
    // gen_goto_tb(s, 0, diff);
    return true;
}

/* Compare and branch (immediate)
 *   31  30         25  24  23                  5 4      0
 * +----+-------------+----+---------------------+--------+
 * | sf | 0 1 1 0 1 0 | op |         imm19       |   Rt   |
 * +----+-------------+----+---------------------+--------+
 */
static bool
disas_comp_b_imm(branch_type_t* s, uint32_t opcode)
{
    // reset_btype(s);

    // match = gen_disas_label(s);
    // tcg_gen_brcondi_i64(op ? TCG_COND_NE : TCG_COND_EQ,
    //                     tcg_cmp, 0, match.label);
    // gen_goto_tb(s, 0, 4);
    // set_disas_label(s, match);
    // gen_goto_tb(s, 1, diff);

    *s = QEMU_Conditional_Branch;
    return true;
}

/* Test and branch (immediate)
 *   31  30         25  24  23   19 18          5 4    0
 * +----+-------------+----+-------+-------------+------+
 * | b5 | 0 1 1 0 1 1 | op |  b40  |    imm14    |  Rt  |
 * +----+-------------+----+-------+-------------+------+
 */
static bool
disas_test_b_imm(branch_type_t* s, uint32_t opcode)
{
    //printf("disas_test_b_imm\n");
    // reset_btype(s);

    // match = gen_disas_label(s);
    // tcg_gen_brcondi_i64(op ? TCG_COND_NE : TCG_COND_EQ,
    //                     tcg_cmp, 0, match.label);
    // gen_goto_tb(s, 0, 4);
    // set_disas_label(s, match);
    // gen_goto_tb(s, 1, diff);
    *s = QEMU_Conditional_Branch;
    return true;
}

/* Conditional branch (immediate)
 *  31           25  24  23                  5   4  3    0
 * +---------------+----+---------------------+----+------+
 * | 0 1 0 1 0 1 0 | o1 |         imm19       | o0 | cond |
 * +---------------+----+---------------------+----+------+
 */
static bool
disas_cond_b_imm(branch_type_t* s, uint32_t opcode)
{
    //printf("disas_cond_b_imm\n");
    unsigned int cond;

    if ((opcode & (1 << 4)) || (opcode & (1 << 24))) {
        // unallocated_encoding(s);
        return false;
    }
    cond = extract32(opcode, 0, 4);

    // reset_btype(s);
    if (cond < 0x0e) {
        /* genuinely conditional branches */
        // DisasLabel match = gen_disas_label(s);
        // arm_gen_test_cc(cond, match.label);
        // gen_goto_tb(s, 0, 4);
        // set_disas_label(s, match);
        // gen_goto_tb(s, 1, diff);
        *s = QEMU_Conditional_Branch;
    } else {
        /* 0xe and 0xf are both "always" conditions */
        // gen_goto_tb(s, 0, diff);
        *s = QEMU_Unconditional_Branch;
    }
    return true;
}

/* CLREX, DSB, DMB, ISB */
static bool
handle_sync(branch_type_t* s, uint32_t opcode,
                        unsigned int op1, unsigned int op2, unsigned int crm)
{
    //printf("handle_sync\n");
    if (op1 != 3) {
        // unallocated_encoding(s);
        return false;
    }

    switch (op2) {
    case 2: /* CLREX */
        // gen_clrex(s, opcode);
        return false;
    case 4: /* DSB */
    case 5: /* DMB */
        return false;
    case 6: /* ISB */
        /* We need to break the TB after this opcode to execute
         * a self-modified code correctly and also to take
         * any pending interrupts immediately.
         */
        // reset_btype(s);
        // gen_goto_tb(s, 0, 4);
        *s = QEMU_Barrier_Branch;
        return true;

    case 7: /* SB */
        if (crm != 0) { // assume dc_isar_feature(aa64_sb, s)
            goto do_unallocated;
        }
        /*
         * TODO: There is no speculation barrier opcode for TCG;
         * MB and end the TB instead.
         */
        // gen_goto_tb(s, 0, 4);
        *s = QEMU_Barrier_Branch;
        return true;

    default:
    do_unallocated:
        // unallocated_encoding(s);
        return false;
    }
}

/* System
 *  31                 22 21  20 19 18 16 15   12 11    8 7   5 4    0
 * +---------------------+---+-----+-----+-------+-------+-----+------+
 * | 1 1 0 1 0 1 0 1 0 0 | L | op0 | op1 |  CRn  |  CRm  | op2 |  Rt  |
 * +---------------------+---+-----+-----+-------+-------+-----+------+
 */
static bool
disas_system(branch_type_t* s, uint32_t opcode)
{
    //printf("disas_system\n");
    unsigned int l, op0, op1, crn, crm, op2, rt;
    l = extract32(opcode, 21, 1);
    op0 = extract32(opcode, 19, 2);
    op1 = extract32(opcode, 16, 3);
    crn = extract32(opcode, 12, 4);
    crm = extract32(opcode, 8, 4);
    op2 = extract32(opcode, 5, 3);
    rt = extract32(opcode, 0, 5);

    if (op0 == 0) {
        if (l || rt != 31) {
            // unallocated_encoding(s);
            return false;
        }
        switch (crn) {
        case 2: /* HINT (including allocated hints like NOP, YIELD, etc) */
            // handle_hint(s, opcode, op1, op2, crm);
            break;
        case 3: /* CLREX, DSB, DMB, ISB */
            return handle_sync(s, opcode, op1, op2, crm);
            break;
        case 4: /* MSR (immediate) */
            //handle_msr_i(s, opcode, op1, op2, crm);
            break;
        default:
            // unallocated_encoding(s);
            break;
        }
        return false;
    }
    // handle_sys(s, opcode, l, op0, op1, op2, crn, crm, rt);
    return false;
}

/* Unconditional branch (register)
 *  31           25 24   21 20   16 15   10 9    5 4     0
 * +---------------+-------+-------+-------+------+-------+
 * | 1 1 0 1 0 1 1 |  opc  |  op2  |  op3  |  Rn  |  op4  |
 * +---------------+-------+-------+-------+------+-------+
 */
static bool
disas_uncond_b_reg(branch_type_t* s, uint32_t opcode)
{
    //printf("disas_uncond_b_reg\n");
    unsigned int opc, op2, op3, op4;

    opc = extract32(opcode, 21, 4);
    op2 = extract32(opcode, 16, 5);
    op3 = extract32(opcode, 10, 6);
    op4 = extract32(opcode, 0, 5);

    if (op2 != 0x1f) {
        goto do_unallocated;
    }

    switch (op3) {
        case 1:
            goto do_unallocated;
            break;
        case 2: /* RETAA, RETAB */
        case 3: /* BRAAZ, BRABZ, BLRAAZ, BLRABZ */
            // pauth is not supported
            goto do_unallocated;
            break;
        default:
            break;
    }

    switch (opc) {
    case 0: /* BR */
        if (op4 != 0) {
            goto do_unallocated;
        }
        *s = QEMU_IndirectReg_Branch;
        return true;
    case 1: /* BLR */
        if (op4 != 0) {
            goto do_unallocated;
        }
        *s = QEMU_IndirectCall_Branch;
        return true;
    case 2: /* RET */
        if (op4 != 0) {
            goto do_unallocated;
        }
        *s = QEMU_Return_Branch;
        return true;

    case 8: /* BRAA */
    case 9: /* BLRAA */
        // pauth is not supported
        goto do_unallocated;
        break;

    case 4: /* ERET */
        if (op4 != 0) {
            goto do_unallocated;
        }
        *s = QEMU_Return_Branch;
        return true;

    case 5: /* DRPS */
        goto do_unallocated;
        break;

    default:
    do_unallocated:
        // unallocated_encoding(s);
        return false;
    }
}

/* Branches, exception generating and system instructions */
static bool
disas_b_exc_sys(branch_type_t* s, uint32_t opcode)
{
    //printf("disas_b_exc_sys\n");
    switch (extract32(opcode, 25, 7)) {
    case 0x0a: case 0x0b:
    case 0x4a: case 0x4b: /* Unconditional branch (immediate) */
        return disas_uncond_b_imm(s, opcode);
        break;
    case 0x1a: case 0x5a: /* Compare & branch (immediate) */
        return disas_comp_b_imm(s, opcode);
        break;
    case 0x1b: case 0x5b: /* Test & branch (immediate) */
        return disas_test_b_imm(s, opcode);
        break;
    case 0x2a: /* Conditional branch (immediate) */
        return disas_cond_b_imm(s, opcode);
        break;
    case 0x6a: /* Exception generation / System */
        if (opcode & (1 << 24)) {
            if (extract32(opcode, 22, 2) == 0) {
                return disas_system(s, opcode);
            } else {
                // unallocated_encoding(s);
                return false;
            }
        } else {
            // disas_exc(s, opcode);
            return false;
        }
        break;
    case 0x6b: /* Unconditional branch (register) */
        return disas_uncond_b_reg(s, opcode);
        break;
    default:
        // unallocated_encoding(s);
        break;
    }
    return false;
}

bool
decode_armv8_branch_opcode(branch_type_t* s, uint32_t opcode)
{
    *s = QEMU_Non_Branch;
    //printf("aarch64_opcode_get_params_branch\n");
switch (extract32(opcode, 25, 4)) {
    case 0x0:
        if (!extract32(opcode, 31, 1)) {
            // unallocated_encoding(s);
        }
        break;
    case 0x1: case 0x3: /* UNALLOCATED */
        // unallocated_encoding(s);
        break;
    case 0x2:
        // if (!disas_sve(s, opcode)) {
        //    unallocated_encoding(s);
        // }
        break;
    case 0x8: case 0x9: /* Data processing - immediate */
        // disas_data_proc_imm(s, opcode);
        break;
    case 0xa: case 0xb: /* Branch, exception generation and system opcodes */
        return disas_b_exc_sys(s, opcode);
        break;
    case 0x4:
    case 0x6:
    case 0xc:
    case 0xe:      /* Loads and stores */
        // disas_ldst(s, opcode);
        break;
    case 0x5:
    case 0xd:      /* Data processing - register */
        // disas_data_proc_reg(s, opcode);
        break;
    case 0x7:
    case 0xf:      /* Data processing - SIMD and floating point */
        // disas_data_proc_simd_fp(s, opcode);
        break;
    default:
        assert(FALSE); /* all 15 cases should be handled above */
        break;
    }

    return false;
}