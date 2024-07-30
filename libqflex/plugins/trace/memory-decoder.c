#include "qemu/osdep.h"
#include "qemu/bitops.h"

#include "trace.h"

/*
 * LDAPR/STLR (unscaled immediate)
 *
 *  31  30            24    22  21       12    10    5     0
 * +------+-------------+-----+---+--------+-----+----+-----+
 * | size | 0 1 1 0 0 1 | opc | 0 |  imm9  | 0 0 | Rn |  Rt |
 * +------+-------------+-----+---+--------+-----+----+-----+
 *
 * Rt: source or destination register
 * Rn: base register
 * imm9: unscaled immediate offset
 * opc: 00: STLUR*, 01/10/11: various LDAPUR*
 * size: size of load/store
 */
static bool disas_ldst_ldapr_stlr(struct mem_access* s, uint32_t opcode)
{
    int opc = extract32(opcode, 22, 2);
    int size = extract32(opcode, 30, 2);
    bool is_store = false;
    bool is_signed = false;

    switch (opc) {
    case 0: /* STLURB */
        is_store = true;
        break;
    case 1: /* LDAPUR* */
        break;
    case 2: /* LDAPURS* 64-bit variant */
        if (size == 3) {
            // unallocated_encoding(s);
            return false;
        }
        is_signed = true;
        break;
    case 3: /* LDAPURS* 32-bit variant */
        if (size > 1) {
            // unallocated_encoding(s);
            return false;
        }
        is_signed = true;
        break;
    default:
        g_assert_not_reached();
    }

    if (is_store) {
        // do_gpr_st(s, cpu_reg(s, rt), clean_addr, mop, true, rt, iss_sf, true);
    } else {
        //do_gpr_ld(s, cpu_reg(s, rt), clean_addr, mop,
        //          extend, true, rt, iss_sf, true);
    }
    *s = (struct mem_access) {.size = size,
          .is_vector = false,
          .is_load = !is_store,
          .is_store = is_store,
          .is_signed = is_signed,
          .is_pair = false,
          .accesses = 1};
    return true;
}

/*
 * Load/Store memory tags
 *
 *  31 30 29         24     22  21     12    10      5      0
 * +-----+-------------+-----+---+------+-----+------+------+
 * | 1 1 | 0 1 1 0 0 1 | op1 | 1 | imm9 | op2 |  Rn  |  Rt  |
 * +-----+-------------+-----+---+------+-----+------+------+
#define LOG2_TAG_GRANULE 4
#define TAG_GRANULE      (1 << LOG2_TAG_GRANULE)
static bool disas_ldst_tag(struct mem_access* s, uint32_t opcode)
{
    int rt = extract32(opcode, 0, 5);
    int rn = extract32(opcode, 5, 5);
    uint64_t offset = sextract64(opcode, 12, 9) << LOG2_TAG_GRANULE;
    int op2 = extract32(opcode, 10, 2);
    int op1 = extract32(opcode, 22, 2);
    bool is_load = false, is_pair = false, is_zero = false, is_mult = false;
    int index = 0;

    // We checked opcode bits [29:24,21] in the caller.
    if (extract32(opcode, 30, 2) != 3) {
        goto do_unallocated;
    }

    // @index is a tri-state variable which has 3 states:
    // < 0 : post-index, writeback
    // = 0 : signed offset
    // > 0 : pre-index, writeback
    switch (op1) {
    case 0:
        if (op2 != 0) {
            // STG
            index = op2 - 2;
        } else {
            // STZGM
            if (s->current_el == 0 || offset != 0) {
                goto do_unallocated;
            }
            is_mult = is_zero = true;
        }
        break;
    case 1:
        if (op2 != 0) {
            // STZG
            is_zero = true;
            index = op2 - 2;
        } else {
            // LDG
            is_load = true;
        }
        break;
    case 2:
        if (op2 != 0) {
            // ST2G
            is_pair = true;
            index = op2 - 2;
        } else {
            // STGM
            if (s->current_el == 0 || offset != 0) {
                goto do_unallocated;
            }
            is_mult = true;
        }
        break;
    case 3:
        if (op2 != 0) {
            // STZ2G
            is_pair = is_zero = true;
            index = op2 - 2;
        } else {
            // LDGM
            if (s->current_el == 0 || offset != 0) {
                goto do_unallocated;
            }
            is_mult = is_load = true;
        }
        break;

    default:
    do_unallocated:
        // unallocated_encoding(s);
        return false;
    }

    if (is_mult) {
        if (is_zero) {
            if (s->ata) {
                // gen_helper_stzgm_tags(cpu_env, addr, tcg_rt);
                // See `mte_helper.c` HELPER(gtzgm_tags) for details
                int log2_dcz_bytes = 7 + 2; // env_archcpu(env)->dcz_blocksize + 2;
                int log2_tag_bytes = log2_dcz_bytes - (LOG2_TAG_GRANULE + 1);
                intptr_t tag_bytes = (intptr_t)1 << log2_tag_bytes;
                memset(mem, tag_pair, tag_bytes);
            }
            // The non-tags portion of STZGM is mostly like DC_ZVA,
            // except the alignment happens before the access.
            gen_helper_dc_zva(cpu_env, clean_addr);
        } else if (s->ata) {
            if (is_load) {
                gen_helper_ldgm(tcg_rt, cpu_env, addr);
            } else {
                gen_helper_stgm(cpu_env, addr, tcg_rt);
            }
        } else {
            MMUAccessType acc = is_load ? MMU_DATA_LOAD : MMU_DATA_STORE;
            int size = 4 << GMID_EL1_BS;
            gen_probe_access(s, clean_addr, acc, size);
        }
        return;
    }

    if (is_load) {
        if (s->ata) {
            gen_helper_ldg(tcg_rt, cpu_env, addr, tcg_rt);
        } else {
            gen_probe_access(s, clean_addr, MMU_DATA_LOAD, MO_8);
        }
    } else {

        if (!s->ata) {
            // For STG and ST2G, we need to check alignment and probe memory.
            // TODO: For STZG and STZ2G, we could rely on the stores below,
            // at least for system mode; user-only won't enforce alignment.
            if (is_pair) {
                gen_helper_st2g_stub(cpu_env, addr);
            } else {
                gen_helper_stg_stub(cpu_env, addr);
            }
        } else if (tb_cflags(s->base.tb) & CF_PARALLEL) {
            if (is_pair) {
                gen_helper_st2g_parallel(cpu_env, addr, tcg_rt);
            } else {
                gen_helper_stg_parallel(cpu_env, addr, tcg_rt);
            }
        } else {
            if (is_pair) {
                gen_helper_st2g(cpu_env, addr, tcg_rt);
            } else {
                gen_helper_stg(cpu_env, addr, tcg_rt);
            }
        }
    }

    if (is_zero) {
        int mem_index = get_mem_index(s);
        int i, n = (1 + is_pair) << LOG2_TAG_GRANULE;

        tcg_gen_qemu_st_i64(tcg_zero, clean_addr, mem_index,
                            MO_UQ | MO_ALIGN_16);
        for (i = 8; i < n; i += 8) {
            tcg_gen_qemu_st_i64(tcg_zero, clean_addr, mem_index, MO_UQ);
        }
    }

    return true;
}
// */

/* AdvSIMD load/store single structure
 *
 *  31  30  29           23 22 21 20       16 15 13 12  11  10 9    5 4    0
 * +---+---+---------------+-----+-----------+-----+---+------+------+------+
 * | 0 | Q | 0 0 1 1 0 1 0 | L R | 0 0 0 0 0 | opc | S | size |  Rn  |  Rt  |
 * +---+---+---------------+-----+-----------+-----+---+------+------+------+
 *
 * AdvSIMD load/store single structure (post-indexed)
 *
 *  31  30  29           23 22 21 20       16 15 13 12  11  10 9    5 4    0
 * +---+---+---------------+-----+-----------+-----+---+------+------+------+
 * | 0 | Q | 0 0 1 1 0 1 1 | L R |     Rm    | opc | S | size |  Rn  |  Rt  |
 * +---+---+---------------+-----+-----------+-----+---+------+------+------+
 *
 * Rt: first (or only) SIMD&FP register to be transferred
 * Rn: base address or SP
 * Rm (post-index only): post-index register (when !31) or size dependent #imm
 * index = encoded in Q:S:size dependent on size
 *
 * lane_size = encoded in R, opc
 * transfer width = encoded in opc, S, size
 */
static bool disas_ldst_single_struct(struct mem_access* s, uint32_t opcode)
{
    int rm = extract32(opcode, 16, 5);
    int size = extract32(opcode, 10, 2);
    int S = extract32(opcode, 12, 1);
    int opc = extract32(opcode, 13, 3);
    int R = extract32(opcode, 21, 1);
    int is_load = extract32(opcode, 22, 1);
    int is_postidx = extract32(opcode, 23, 1);
    int is_q = extract32(opcode, 30, 1);

    int scale = extract32(opc, 1, 2);
    int selem = (extract32(opc, 0, 1) << 1 | R) + 1;
    bool replicate = false;
    int index = is_q << 3 | S << 2 | size;
    int xs;

    if (extract32(opcode, 31, 1)) {
        // unallocated_encoding(s);
        return false;
    }
    if (!is_postidx && rm != 0) {
        // unallocated_encoding(s);
        return false;
    }

    switch (scale) {
    case 3:
        if (!is_load || S) {
            // unallocated_encoding(s);
            return false;
        }
        scale = size;
        replicate = true;
        break;
    case 0:
        break;
    case 1:
        if (extract32(size, 0, 1)) {
            // unallocated_encoding(s);
            return false;
        }
        index >>= 1;
        break;
    case 2:
        if (extract32(size, 1, 1)) {
            // unallocated_encoding(s);
            return false;
        }
        if (!extract32(size, 0, 1)) {
            index >>= 2;
        } else {
            if (S) {
                // unallocated_encoding(s);
                return false;
            }
            index >>= 3;
            scale = 3;
        }
        break;
    default:
        g_assert_not_reached();
    }

    for (xs = 0; xs < selem; xs++) {
        if (replicate) {
            /* Load and replicate to all elements */
            // For more details check in `tcg/tcg-op-gvec.c`:
            // do_dup(scale, vec_full_reg_offset(s, rt), (is_q + 1) * 8, vec_full_reg_size(s), tcg_tmp, NULL, 0);
        } else {
            /* Load/store one element per register */
            if (is_load) {
                // do_vec_ld(s, rt, index, clean_addr, mop);
            } else {
                // do_vec_st(s, rt, index, clean_addr, mop);
            }
        }
    }
    *s = (struct mem_access) {.size = scale,
          .is_vector = false,
          .is_load = is_load,
          .is_store = !is_load,
          .is_signed = false,
          .is_pair = false,
          .accesses = selem};
    return true;
}

/*
 * LDNP (Load Pair - non-temporal hint)
 * LDP (Load Pair - non vector)
 * LDPSW (Load Pair Signed Word - non vector)
 * STNP (Store Pair - non-temporal hint)
 * STP (Store Pair - non vector)
 * LDNP (Load Pair of SIMD&FP - non-temporal hint)
 * LDP (Load Pair of SIMD&FP)
 * STNP (Store Pair of SIMD&FP - non-temporal hint)
 * STP (Store Pair of SIMD&FP)
 *
 *  31 30 29   27  26  25 24   23  22 21   15 14   10 9    5 4    0
 * +-----+-------+---+---+-------+---+-----------------------------+
 * | opc | 1 0 1 | V | 0 | index | L |  imm7 |  Rt2  |  Rn  | Rt   |
 * +-----+-------+---+---+-------+---+-------+-------+------+------+
 *
 * opc: LDP/STP/LDNP/STNP        00 -> 32 bit, 10 -> 64 bit
 *      LDPSW/STGP               01
 *      LDP/STP/LDNP/STNP (SIMD) 00 -> 32 bit, 01 -> 64 bit, 10 -> 128 bit
 *   V: 0 -> GPR, 1 -> Vector
 * idx: 00 -> signed offset with non-temporal hint, 01 -> post-index,
 *      10 -> signed offset, 11 -> pre-index
 *   L: 0 -,> Store 1 -> Load
 *
 * Rt, Rt2 = GPR or SIMD registers to be stored
 * Rn = general purpose register containing address
 * imm7 = signed offset (multiple of 4 or 8 depending on size)
 */
static bool disas_ldst_pair(struct mem_access* s, uint32_t opcode)
{
    int index = extract32(opcode, 23, 2);
    bool is_vector = extract32(opcode, 26, 1);
    bool is_load = extract32(opcode, 22, 1);
    int opc = extract32(opcode, 30, 2);

    bool is_signed = false;
    bool set_tag = false;

    int size;

    if (opc == 3) {
        // unallocated_encoding(s);
        return false;
    }

    if (is_vector) {
        size = 2 + opc;
    } else if (opc == 1 && !is_load) {
        /* STGP */
        size = 3;
        set_tag = true;
    } else {
        size = 2 + extract32(opc, 1, 1);
        is_signed = extract32(opc, 0, 1);
        if (!is_load && is_signed) {
            // unallocated_encoding(s);
            return false;
        }
    }

    switch (index) {
    case 1: /* post-index */
        break;
    case 0:
        /* signed offset with "non-temporal" hint. Since we don't emulate
         * caches we don't care about hints to the cache system about
         * data access patterns, and handle this identically to plain
         * signed offset.
         */
        if (is_signed) {
            /* There is no non-temporal-hint version of LDPSW */
            // unallocated_encoding(s);
            return false;
        }
        break;
    case 2: /* signed offset, rn not updated */
        break;
    case 3: /* pre-index */
        break;
    }

    if (set_tag) {
        // Unsupported
        printf("ERROR:QFlex, We do not support memory callbacks for tags.");
        /*
        if (!s->ata) {
            gen_helper_stg_stub(cpu_env, dirty_addr);
            *s = (struct mem_access) {.size = size,
                  .is_vector = false,
                  .is_load = false,
                  .is_store = true,
                  .is_signed = false,
                  .is_pair = true,
                  .accesses = 2};
        } else if (tb_cflags(s->base.tb) & CF_PARALLEL) {
            gen_helper_stg_parallel(cpu_env, dirty_addr, dirty_addr);
        } else {
            gen_helper_stg(cpu_env, dirty_addr, dirty_addr);
        }
        */
    }
    *s = (struct mem_access) {.size = size,
          .is_vector = is_vector,
          .is_load = is_load,
          .is_store = !is_load,
          .is_signed = false,
          .is_pair = true,
          .accesses = 2};
    if (is_vector) {
        if (is_load) {
            // do_fp_ld(s, rt, clean_addr, size);
        } else {
            // do_fp_st(s, rt, clean_addr, size);
        }
        if (is_load) {
            // do_fp_ld(s, rt2, clean_addr, size);
        } else {
            // do_fp_st(s, rt2, clean_addr, size);
        }
    } else {
        if (is_load) {
            /* Do not modify tcg_rt before recognizing any exception
             * from the second load.
             */
            // do_gpr_ld(s, tmp, clean_addr, size + is_signed * MO_SIGN,
            //           false, false, 0, false, false);
            // do_gpr_ld(s, tcg_rt2, clean_addr, size + is_signed * MO_SIGN,
            //           false, false, 0, false, false);
            *s = (struct mem_access) {.size = size,
                  .is_vector = is_vector,
                  .is_load = true,
                  .is_store = false,
                  .is_signed = is_signed,
                  .is_pair = true,
                  .accesses = 2};
        } else {
            // do_gpr_st(s, tcg_rt, clean_addr, size,
            //           false, 0, false, false);
            // do_gpr_st(s, tcg_rt2, clean_addr, size,
            //           false, 0, false, false);
        }
    }

    return true;
}

/*
 * Load register (literal)
 *
 *  31 30 29   27  26 25 24 23                5 4     0
 * +-----+-------+---+-----+-------------------+-------+
 * | opc | 0 1 1 | V | 0 0 |     imm19         |  Rt   |
 * +-----+-------+---+-----+-------------------+-------+
 *
 * V: 1 -> vector (simd/fp)
 * opc (non-vector): 00 -> 32 bit, 01 -> 64 bit,
 *                   10-> 32 bit signed, 11 -> prefetch
 * opc (vector): 00 -> 32 bit, 01 -> 64 bit, 10 -> 128 bit (11 unallocated)
 */
static bool disas_ld_lit(struct mem_access* s, uint32_t opcode)
{
    bool is_vector = extract32(opcode, 26, 1);
    int opc = extract32(opcode, 30, 2);
    bool is_signed = false;
    int size = 2;

    if (is_vector) {
        if (opc == 3) {
            // unallocated_encoding(s);
            return false;
        }
        size = 2 + opc;
    } else {
        if (opc == 3) {
            /* PRFM (literal) : prefetch */
            return false;
        }
        size = 2 + extract32(opc, 0, 1);
        is_signed = extract32(opc, 1, 1);
    }

    if (is_vector) {
        // do_fp_ld(s, rt, clean_addr, size);
    } else {
        /* Only unsigned 32bit loads target 32bit registers.  */
        // do_gpr_ld(s, tcg_rt, clean_addr, size + is_signed * MO_SIGN,
        //           false, true, rt, iss_sf, false);
    }
    *s = (struct mem_access) {.size = size,
          .is_vector = is_vector,
          .is_load = true,
          .is_store = false,
          .is_signed = is_signed,
          .is_pair = false,
          .accesses = 1};
    return true;
}

/* Load/store exclusive
 *
 *  31 30 29         24  23  22   21  20  16  15  14   10 9    5 4    0
 * +-----+-------------+----+---+----+------+----+-------+------+------+
 * | sz  | 0 0 1 0 0 0 | o2 | L | o1 |  Rs  | o0 |  Rt2  |  Rn  | Rt   |
 * +-----+-------------+----+---+----+------+----+-------+------+------+
 *
 *  sz: 00 -> 8 bit, 01 -> 16 bit, 10 -> 32 bit, 11 -> 64 bit
 *   L: 0 -> store, 1 -> load
 *  o2: 0 -> exclusive, 1 -> not
 *  o1: 0 -> single register, 1 -> register pair
 *  o0: 1 -> load-acquire/store-release, 0 -> not
 */
static bool disas_ldst_excl(struct mem_access* s, uint32_t opcode)
{
    int rt = extract32(opcode, 0, 5);
    int rt2 = extract32(opcode, 10, 5);
    int rs = extract32(opcode, 16, 5);
    int is_lasr = extract32(opcode, 15, 1);
    int o2_L_o1_o0 = extract32(opcode, 21, 3) * 2 | is_lasr;
    int size = extract32(opcode, 30, 2);

    switch (o2_L_o1_o0) {
    case 0x0: /* STXR */
    case 0x1: /* STLXR */
        // gen_store_exclusive(s, rs, rt, rt2, clean_addr, size, false);
        *s = (struct mem_access) {.size = size,
          .is_vector = false,
          .is_load = true,
          .is_store = true,
          .is_signed = false,
          .is_pair = false,
          .accesses = 2};
        return true;

    case 0x4: /* LDXR */
    case 0x5: /* LDAXR */
        // gen_load_exclusive(s, rt, rt2, clean_addr, size, false);
        *s = (struct mem_access) {.size = size,
          .is_vector = false,
          .is_load = true,
          .is_store = false,
          .is_signed = false,
          .is_pair = false,
          .accesses = 1};
        return true;

    case 0x8: /* STLLR */
        /* StoreLORelease is the same as Store-Release for QEMU.  */
        /* fall through */
    case 0x9: /* STLR */
        /* Generate ISS for non-exclusive accesses including LASR.  */
        /* TODO: ARMv8.4-LSE SCTLR.nAA */
        // do_gpr_st(s, cpu_reg(s, rt), clean_addr, size | MO_ALIGN, true, rt,
        //           disas_ldst_compute_iss_sf(size, false, 0), is_lasr);
        *s = (struct mem_access) {.size = size,
          .is_vector = false,
          .is_load = true,
          .is_store = true,
          .is_signed = false,
          .is_pair = false,
          .accesses = 2};
        return true;

    case 0xc: /* LDLAR */
        /* LoadLOAcquire is the same as Load-Acquire for QEMU.  */
        /* fall through */
    case 0xd: /* LDAR */
        /* Generate ISS for non-exclusive accesses including LASR.  */
        /* TODO: ARMv8.4-LSE SCTLR.nAA */
        // do_gpr_ld(s, cpu_reg(s, rt), clean_addr, size | MO_ALIGN, false, true,
        //           rt, disas_ldst_compute_iss_sf(size, false, 0), is_lasr);
        *s = (struct mem_access) {.size = size,
          .is_vector = false,
          .is_load = true,
          .is_store = false,
          .is_signed = false,
          .is_pair = false,
          .accesses = 1};
        return true;

    case 0x2: case 0x3: /* CASP / STXP */
        if (size & 2) { /* STXP / STLXP */
            // gen_store_exclusive(s, rs, rt, rt2, clean_addr, size, true);
            if (size == 2) {
                // Must access 2x32 bits in single access (64 bits)
                *s = (struct mem_access) {.size = 3,
                      .is_vector = false,
                      .is_load = true,
                      .is_store = true,
                      .is_signed = false,
                      .is_pair = true,
                      .accesses = 2};
            } else {
                // Must access 2x64 bits in two access
                *s = (struct mem_access) {.size = size,
                      .is_vector = false,
                      .is_load = true,
                      .is_store = true,
                      .is_signed = false,
                      .is_pair = true,
                      .accesses = 4};
            }
            return true;
        }
        if (rt2 == 31
            && ((rt | rs) & 1) == 0) {
            /* CASP / CASPL */
            // gen_compare_and_swap_pair(s, rs, rt, rn, size | 2);
            *s = (struct mem_access) {.size = size | 2,
                  .is_vector = false,
                  .is_load = true,
                  .is_store = true,
                  .is_signed = false,
                  .is_pair = true,
                  .accesses = 4};
            return true;
        }
        break;

    case 0x6: case 0x7: /* CASPA / LDXP */
        if (size & 2) { /* LDXP / LDAXP */
            // addr = r[n]
            // gen_load_exclusive(s, rt, rt2, clean_addr, size, true);
            if (size == 2) {
                // Must access 2x32 bits in single access (64 bits)
                *s = (struct mem_access) {.size = 3,
                      .is_vector = false,
                      .is_load = true,
                      .is_store = false,
                      .is_signed = false,
                      .is_pair = true,
                      .accesses = 1};
            } else {
                // Must access 2x64 bits in two access
                *s = (struct mem_access) {.size = size,
                      .is_vector = false,
                      .is_load = true,
                      .is_store = false,
                      .is_signed = false,
                      .is_pair = true,
                      .accesses = 2};
            }
            return true;
        }
        if (rt2 == 31
            && ((rt | rs) & 1) == 0) {
            /* CASPA / CASPAL */
            // gen_compare_and_swap_pair(s, rs, rt, rn, size | 2);
            *s = (struct mem_access) {.size = size | 2,
                  .is_vector = false,
                  .is_load = true,
                  .is_store = true,
                  .is_signed = false,
                  .is_pair = true,
                  .accesses = 4};
            return true;
        }
        break;

    case 0xa: /* CAS */
    case 0xb: /* CASL */
    case 0xe: /* CASA */
    case 0xf: /* CASAL */
        if (rt2 == 31) { // Assume dc_isar_feature(aa64_atomics, s) == true
            // gen_compare_and_swap(s, rs, rt, rn, size);
            *s = (struct mem_access) {.size = size,
                  .is_vector = false,
                  .is_load = true,
                  .is_store = true,
                  .is_signed = false,
                  .is_pair = false,
                  .accesses = 2};
            return true;
        }
        break;
    }
    // unallocated_encoding(s);
    return false;
}

/*
 * PAC memory operations
 *
 *  31  30      27  26    24    22  21       12  11  10    5     0
 * +------+-------+---+-----+-----+---+--------+---+---+----+-----+
 * | size | 1 1 1 | V | 0 0 | M S | 1 |  imm9  | W | 1 | Rn |  Rt |
 * +------+-------+---+-----+-----+---+--------+---+---+----+-----+
 *
 * Rt: the result register
 * Rn: base address or SP
 * V: vector flag (always 0 as of v8.3)
 * M: clear for key DA, set for key DB
 * W: pre-indexing flag
 * S: sign for imm9.
 */
static bool disas_ldst_pac(struct mem_access* s, uint32_t opcode,
                           int size, int rt, bool is_vector)
{
    if (size != 3 || is_vector) { // Assume `dc_isar_feature(aa64_pauth, s) == true`
        // unallocated_encoding(s);
        return false;
    }

    /* Form the 10-bit signed, scaled offset.  */

    // do_gpr_ld(s, tcg_rt, clean_addr, size,
    //           /* extend */ false, /* iss_valid */ !is_wback,
    //           /* iss_srt */ rt, /* iss_sf */ true, /* iss_ar */ false);
    *s = (struct mem_access) {.size = size,
          .is_vector = false,
          .is_load = true,
          .is_store = false,
          .is_signed = false,
          .is_pair = false,
          .accesses = 1};
    return true;
}

/*
 * Load/store (register offset)
 *
 * 31 30 29   27  26 25 24 23 22 21  20  16 15 13 12 11 10 9  5 4  0
 * +----+-------+---+-----+-----+---+------+-----+--+-----+----+----+
 * |size| 1 1 1 | V | 0 0 | opc | 1 |  Rm  | opt | S| 1 0 | Rn | Rt |
 * +----+-------+---+-----+-----+---+------+-----+--+-----+----+----+
 *
 * For non-vector:
 *   size: 00-> byte, 01 -> 16 bit, 10 -> 32bit, 11 -> 64bit
 *   opc: 00 -> store, 01 -> loadu, 10 -> loads 64, 11 -> loads 32
 * For vector:
 *   size is opc<1>:size<1:0> so 100 -> 128 bit; 110 and 111 unallocated
 *   opc<0>: 0 -> store, 1 -> load
 * V: 1 -> vector/simd
 * opt: extend encoding (see DecodeRegExtend)
 * S: if S=1 then scale (essentially index by sizeof(size))
 * Rt: register to transfer into/out of
 * Rn: address register or SP for base
 * Rm: offset register or ZR for offset
 */
static bool disas_ldst_reg_roffset(struct mem_access* s, uint32_t opcode,
                                   int opc,
                                   int size,
                                   int rt,
                                   bool is_vector)
{
    int opt = extract32(opcode, 13, 3);
    bool is_signed = false;
    bool is_store = false;

    if (extract32(opt, 1, 1) == 0) {
        // unallocated_encoding(s);
        return false;
    }

    if (is_vector) {
        size |= (opc & 2) << 1;
        if (size > 4) {
            // unallocated_encoding(s);
            return false;
        }
        is_store = !extract32(opc, 0, 1);
    } else {
        if (size == 3 && opc == 2) {
            /* PRFM - prefetch */
            return false;
        }
        if (opc == 3 && size > 1) {
            // unallocated_encoding(s);
            return false;
        }
        is_store = (opc == 0);
        is_signed = extract32(opc, 1, 1);
    }

    if (is_vector) {
        if (is_store) {
            // do_fp_st(s, rt, clean_addr, size);
        } else {
            // do_fp_ld(s, rt, clean_addr, size);
        }
    } else {
        if (is_store) {
            // do_gpr_st(s, tcg_rt, clean_addr, size,
            //           true, rt, iss_sf, false);
        } else {
            // do_gpr_ld(s, tcg_rt, clean_addr, size + is_signed * MO_SIGN,
            //           is_extended, true, rt, iss_sf, false);
        }
    }
    *s = (struct mem_access) {.size = size,
          .is_vector = is_vector,
          .is_load = !is_store,
          .is_store = is_store,
          .is_signed = is_signed,
          .is_pair = false,
          .accesses = 1};
    return true;
}


/* Atomic memory operations
 *
 *  31  30      27  26    24    22  21   16   15    12    10    5     0
 * +------+-------+---+-----+-----+---+----+----+-----+-----+----+-----+
 * | size | 1 1 1 | V | 0 0 | A R | 1 | Rs | o3 | opc | 0 0 | Rn |  Rt |
 * +------+-------+---+-----+-----+--------+----+-----+-----+----+-----+
 *
 * Rt: the result register
 * Rn: base address or SP
 * Rs: the source register for the operation
 * V: vector flag (always 0 as of v8.3)
 * A: acquire flag
 * R: release flag
 */
enum {
  ATOMIC_ADD,
  ATOMIC_AND,
  ATOMIC_XOR,
  ATOMIC_OR,
  ATOMIC_SMAX,
  ATOMIC_SMIN,
  ATOMIC_UMAX,
  ATOMIC_UMIN,
  ATOMIC_XCHG,
  ATOMIC_UNKNOWN
};

static bool disas_ldst_atomic(struct mem_access* s, uint32_t opcode,
                              int size, int rt, bool is_vector)
{
    int rs = extract32(opcode, 16, 5);
    int o3_opc = extract32(opcode, 12, 4);
    bool r = extract32(opcode, 22, 1);
    bool a = extract32(opcode, 23, 1);
    int fn __attribute__((unused)) = ATOMIC_UNKNOWN;
    bool is_signed = false;

    if (is_vector) {
        // unallocated_encoding(s);
        return false;
    }
    switch (o3_opc) {
    case 000: /* LDADD */
        fn = ATOMIC_ADD;
        break;
    case 001: /* LDCLR */
        fn = ATOMIC_AND;
        break;
    case 002: /* LDEOR */
        fn = ATOMIC_XOR;
        break;
    case 003: /* LDSET */
        fn = ATOMIC_OR;
        break;
    case 004: /* LDSMAX */
        fn = ATOMIC_SMAX;
        is_signed = true;
        break;
    case 005: /* LDSMIN */
        fn = ATOMIC_SMIN;
        is_signed = true;
        break;
    case 006: /* LDUMAX */
        fn = ATOMIC_UMAX;
        break;
    case 007: /* LDUMIN */
        fn = ATOMIC_UMIN;
        break;
    case 010: /* SWP */
        fn = ATOMIC_XCHG;
        break;
    case 014: /* LDAPR, LDAPRH, LDAPRB */
        if (rs != 31 || a != 1 || r != 0) {
            // unallocated_encoding(s);
            return false;
        }
        break;
    default:
        // unallocated_encoding(s);
        return false;
    }

    if (o3_opc == 014) {
        // do_gpr_ld(s, cpu_reg(s, rt), clean_addr, size, false,
        //           true, rt, disas_ldst_compute_iss_sf(size, false, 0), true);
        *s = (struct mem_access) {.size = size,
          .is_vector = false,
          .is_load = true,
          .is_store = false,
          .is_signed = false,
          .is_pair = false,
          .accesses = 1};
        return true;
    }

    // fn(tcg_rt, clean_addr, tcg_rs, get_mem_index(s), mop);
    *s = (struct mem_access) {.size = size,
          .is_vector = false,
          .is_load = true,
          .is_store = true,
          .is_signed = is_signed,
          .is_pair = false,
          .accesses = 2}; // Given that it's Aquire + Release, is it 2 access?
    return true;
}

/* AdvSIMD load/store multiple structures
 *
 *  31  30  29           23 22  21         16 15    12 11  10 9    5 4    0
 * +---+---+---------------+---+-------------+--------+------+------+------+
 * | 0 | Q | 0 0 1 1 0 0 0 | L | 0 0 0 0 0 0 | opcode | size |  Rn  |  Rt  |
 * +---+---+---------------+---+-------------+--------+------+------+------+
 *
 * AdvSIMD load/store multiple structures (post-indexed)
 *
 *  31  30  29           23 22  21  20     16 15    12 11  10 9    5 4    0
 * +---+---+---------------+---+---+---------+--------+------+------+------+
 * | 0 | Q | 0 0 1 1 0 0 1 | L | 0 |   Rm    | opcode | size |  Rn  |  Rt  |
 * +---+---+---------------+---+---+---------+--------+------+------+------+
 *
 * Rt: first (or only) SIMD&FP register to be transferred
 * Rn: base address or SP
 * Rm (post-index only): post-index register (when !31) or size dependent #imm
 */
static bool disas_ldst_multiple_struct(struct mem_access* s, uint32_t opcode)
{
    int rm = extract32(opcode, 16, 5);
    int size = extract32(opcode, 10, 2);
    uint32_t fn = extract32(opcode, 12, 4);
    bool is_store = !extract32(opcode, 22, 1);
    bool is_postidx = extract32(opcode, 23, 1);
    bool is_q = extract32(opcode, 30, 1);

    int elements; /* elements per vector */
    int rpt;    /* num iterations */
    int selem;  /* structure elements */
    int r;

    if (extract32(opcode, 31, 1) || extract32(opcode, 21, 1)) {
        // unallocated_encoding(s);
        return false;
    }

    if (!is_postidx && rm != 0) {
        // unallocated_encoding(s);
        return false;
    }

    /* From the shared decode logic */
    switch (fn) {
    case 0x0:
        rpt = 1;
        selem = 4;
        break;
    case 0x2:
        rpt = 4;
        selem = 1;
        break;
    case 0x4:
        rpt = 1;
        selem = 3;
        break;
    case 0x6:
        rpt = 3;
        selem = 1;
        break;
    case 0x7:
        rpt = 1;
        selem = 1;
        break;
    case 0x8:
        rpt = 1;
        selem = 2;
        break;
    case 0xa:
        rpt = 2;
        selem = 1;
        break;
    default:
        // unallocated_encoding(s);
        return false;
    }

    if (size == 3 && !is_q && selem != 1) {
        /* reserved */
        // unallocated_encoding(s);
        return false;
    }

    /*
     * Consecutive little-endian elements from a single register
     * can be promoted to a larger little-endian operation.
     */
    if (selem == 1) { // Assume (endian == MO_LE)
        size = 3;
    }

    elements = (is_q ? 16 : 8) >> size;
    for (r = 0; r < rpt; r++) {
        int e;
        for (e = 0; e < elements; e++) {
            int xs;
            for (xs = 0; xs < selem; xs++) {
                if (is_store) {
                    // do_vec_st(s, tt, e, clean_addr, mop);
                } else {
                    // do_vec_ld(s, tt, e, clean_addr, mop);
                }
            }
        }
    }
    *s = (struct mem_access) {.size = size,
          .is_vector = true,
          .is_load = !is_store,
          .is_store = is_store,
          .is_signed = false,
          .is_pair = false,
          .accesses = rpt * elements * selem};
    return true;
}

/*
 * Load/store (immediate post-indexed)
 * Load/store (immediate pre-indexed)
 * Load/store (unscaled immediate)
 *
 * 31 30 29   27  26 25 24 23 22 21  20    12 11 10 9    5 4    0
 * +----+-------+---+-----+-----+---+--------+-----+------+------+
 * |size| 1 1 1 | V | 0 0 | opc | 0 |  imm9  | idx |  Rn  |  Rt  |
 * +----+-------+---+-----+-----+---+--------+-----+------+------+
 *
 * idx = 01 -> post-indexed, 11 pre-indexed, 00 unscaled imm. (no writeback)
         10 -> unprivileged
 * V = 0 -> non-vector
 * size: 00 -> 8 bit, 01 -> 16 bit, 10 -> 32 bit, 11 -> 64bit
 * opc: 00 -> store, 01 -> loadu, 10 -> loads 64, 11 -> loads 32
 */
static bool disas_ldst_reg_imm9(struct mem_access* s, uint32_t opcode,
                                int opc,
                                int size,
                                int rt,
                                bool is_vector)
{
    int idx = extract32(opcode, 10, 2);
    bool is_signed = false;
    bool is_store = false;
    bool is_unpriv = (idx == 2);

    if (is_vector) {
        size |= (opc & 2) << 1;
        if (size > 4 || is_unpriv) {
            // unallocated_encoding(s);
            return false;
        }
        is_store = ((opc & 1) == 0);
    } else {
        if (size == 3 && opc == 2) {
            /* PRFM - prefetch */
            if (idx != 0) {
                // unallocated_encoding(s);
                return false;
            }
            return false;
        }
        if (opc == 3 && size > 1) {
            // unallocated_encoding(s);
            return false;
        }
        is_store = (opc == 0);
        is_signed = extract32(opc, 1, 1);
    }

    switch (idx) {
    case 0:
    case 2:
        break;
    case 1:
        break;
    case 3:
        break;
    default:
        g_assert_not_reached();
    }

    *s = (struct mem_access) {.size = size,
          .is_vector = is_vector,
          .is_load = !is_store,
          .is_store = is_store,
          .is_signed = is_signed,
          .is_pair = false,
          .accesses = 1};
    return true;
}

/*
 * Load/store (unsigned immediate)
 *
 * 31 30 29   27  26 25 24 23 22 21        10 9     5
 * +----+-------+---+-----+-----+------------+-------+------+
 * |size| 1 1 1 | V | 0 1 | opc |   imm12    |  Rn   |  Rt  |
 * +----+-------+---+-----+-----+------------+-------+------+
 *
 * For non-vector:
 *   size: 00-> byte, 01 -> 16 bit, 10 -> 32bit, 11 -> 64bit
 *   opc: 00 -> store, 01 -> loadu, 10 -> loads 64, 11 -> loads 32
 * For vector:
 *   size is opc<1>:size<1:0> so 100 -> 128 bit; 110 and 111 unallocated
 *   opc<0>: 0 -> store, 1 -> load
 * Rn: base address register (inc SP)
 * Rt: target register
 */
static bool disas_ldst_reg_unsigned_imm(struct mem_access* s, uint32_t opcode,
                                        int opc,
                                        int size,
                                        int rt,
                                        bool is_vector)
{
    bool is_store;
    bool is_signed = false;

    if (is_vector) {
        size |= (opc & 2) << 1;
        if (size > 4) {
            // unallocated_encoding(s);
            return false;
        }
        is_store = !extract32(opc, 0, 1);
    } else {
        if (size == 3 && opc == 2) {
            /* PRFM - prefetch */
            return false;
        }
        if (opc == 3 && size > 1) {
            // unallocated_encoding(s);
            return false;
        }
        is_store = (opc == 0);
        is_signed = extract32(opc, 1, 1);
    }

   if (is_vector) {
        if (is_store) {
            // do_fp_st(s, rt, clean_addr, size);
        } else {
            // do_fp_ld(s, rt, clean_addr, size);
        }
    } else {
        if (is_store) {
            // do_gpr_st(s, tcg_rt, clean_addr, size,
            //           true, rt, iss_sf, false);
        } else {
            // do_gpr_ld(s, tcg_rt, clean_addr, size + is_signed * MO_SIGN,
            //           is_extended, true, rt, iss_sf, false);
        }
    }

    *s = (struct mem_access) {.size = size,
          .is_vector = is_vector,
          .is_load = !is_store,
          .is_store = is_store,
          .is_signed = is_signed,
          .is_pair = false,
          .accesses = 1};
    return true;
}

/* Load/store register (all forms) */
static bool disas_ldst_reg(struct mem_access* s, uint32_t opcode)
{
    int rt = extract32(opcode, 0, 5);
    int opc = extract32(opcode, 22, 2);
    bool is_vector = extract32(opcode, 26, 1);
    int size = extract32(opcode, 30, 2);

    switch (extract32(opcode, 24, 2)) {
    case 0:
        if (extract32(opcode, 21, 1) == 0) {
            /* Load/store register (unscaled immediate)
             * Load/store immediate pre/post-indexed
             * Load/store register unprivileged
             */
            return disas_ldst_reg_imm9(s, opcode, opc, size, rt, is_vector);
        }
        switch (extract32(opcode, 10, 2)) {
        case 0:
            return disas_ldst_atomic(s, opcode, size, rt, is_vector);
        case 2:
            return disas_ldst_reg_roffset(s, opcode, opc, size, rt, is_vector);
        default:
            return disas_ldst_pac(s, opcode, size, rt, is_vector);
        }
        break;
    case 1:
        return disas_ldst_reg_unsigned_imm(s, opcode, opc, size, rt, is_vector);
    }
    // unallocated_encoding(s);
    return false;
}

static bool
disas_ldst(struct mem_access* s, uint32_t opcode)
{
    bool has_mem_access = false;
    switch (extract32(opcode, 24, 6)) {
    case 0x08: /* Load/store exclusive */
        has_mem_access = disas_ldst_excl(s, opcode);
        break;
    case 0x18: case 0x1c: /* Load register (literal) */
        has_mem_access = disas_ld_lit(s, opcode);
        break;
    case 0x28: case 0x29:
    case 0x2c: case 0x2d: /* Load/store pair (all forms) */
        has_mem_access = disas_ldst_pair(s, opcode);
        break;
    case 0x38: case 0x39:
    case 0x3c: case 0x3d: /* Load/store register (all forms) */
        has_mem_access = disas_ldst_reg(s, opcode);
        break;
    case 0x0c: /* AdvSIMD load/store multiple structures */
        has_mem_access = disas_ldst_multiple_struct(s, opcode);
        break;
    case 0x0d: /* AdvSIMD load/store single structure */
        has_mem_access = disas_ldst_single_struct(s, opcode);
        break;
    case 0x19:
        if (extract32(opcode, 21, 1) != 0) {
            printf("ERROR:QFlex, We do not support memory callbacks for tags.\n");
            // has_mem_access = disas_ldst_tag(s, opcode);
            return false;
        } else if (extract32(opcode, 10, 2) == 0) {
            has_mem_access = disas_ldst_ldapr_stlr(s, opcode);
        } else {
            // unallocated_encoding(s);
            has_mem_access = false;
        }
        break;
    default:
        //unallocated_encoding(s);
        has_mem_access = false;
        break;
    }
    return has_mem_access;
}

/*
    * Link to branches and system instructions:
    * https://developer.arm.com/documentation/ddi0602/2024-03/Index-by-Encoding/Branches--Exception-Generating-and-System-instructions
*/

static bool
disas_branch_sys(struct mem_access* s, uint32_t opcode)
{
    bool has_mem_access = false;

    size_t size = 0;                // 10 for 32-bit and 11 for 64-bit
    bool is_atomic = false;         // Is atomic instruction?
    bool is_store = false;          // Is load/store instruction?

    switch(extract32(opcode, 29, 3)) {
        case 0x2:       /* Conditional/Misc branch */
            break;
        case 0x6:       /* Exception handling and system instructions */
            switch(extract32(opcode, 24, 2)) {
                case 0x0:       /* Exception generation */
                    break;
                case 0x1:
                    switch(extract32(opcode, 12, 12)) {
                        case 0x31:  /* Wait for timer interrrupt */
                        case 0x32:  /* Hints to compiler and special instructions like NOPs (TODO: may need to be checked in detail) */
                        case 0x33:  /* Barriers */
                        case 0x4: case 0x14: case 0x24: case 0x34: case 0x44: case 0x54: case 0x64: case 0x74: /* For controlling processor flags */   
                            break;
                        default:
                            switch(extract32(opcode, 19, 5)) {
                                case 0x4:   /* Transcation start and end */
                                    break;
                                case 0x1: case 0x5: /* System instructions */
                                    switch(extract32(opcode, 12, 4)) {
                                        case 0x7:   /* Data Cache and Instr. Cache operation */
                                            switch(extract32(opcode, 8, 4)) {   /* CRm field */
                                                case 0x1:
                                                case 0x4:
                                                case 0x5:
                                                case 0x6:
                                                case 0xa:
                                                case 0xb:
                                                case 0xc:
                                                case 0xd:
                                                    has_mem_access = true;
                                                    is_store = true;
                                                    size = 2;       /* Everything should be 32-bit */
                                                    break;
                                                default:
                                                    printf("[Error]:QFlex, Unallocated encoding.\n");
                                                    g_assert_not_reached();
                                                    break;
                                            }
                                            break;
                                        default:
                                            break;
                                    }

                                break;
                            }
                            break;
                    }
                    break;
                case 0x2:       /* Unconditional branch */
                case 0x3:
                    break;
            }
            break;
        case 0x0:       /* Unconditional branch */
        case 0x4:
            break;
        case 0x1:       /* Compare and branch */
        case 0x5:
            break;
        case 0x3:       /* Unallocated */
        case 0x7:
            break;
    }

    *s = (struct mem_access) {.size = size,
        .is_vector = true,
        .is_load = !is_store,
        .is_store = is_store,
        .is_signed = false,         // TODO: Not handled for now because unused
        .is_pair = false,
        .is_atomic = is_atomic,
        .accesses = 1};             // TODO: Not handled for now because unused

    return has_mem_access;
}

/* 
    * Link to SVE instructions:
    * https://developer.arm.com/documentation/ddi0602/2024-03/Index-by-Encoding/SVE-encodings
*/
static bool
disas_sve(struct mem_access* s, uint32_t opcode)
{
    bool has_mem_access = false;
    // Only handling these three fields as others not used
    size_t size = 0;                // 10 for 32-bit and 11 for 64-bit
    bool is_atomic = false;         // Is atomic instruction? False for all SVE memory accesses
    bool is_store = false;          // Is load/store instruction?

    switch(extract32(opcode, 29, 3)) {
        case 0x4:
            has_mem_access = true;  /* SVE Memory - 32-bit Gather and Unsized Contiguous */
            is_store = false;       /* All Prefetch (Load) instructions */
            g_assert(extract32(opcode, 25, 4) == 0x2);
            switch(extract32(opcode, 23, 2)) {
                case 0x3:           /* LDR, and SVE prefetch */
                    switch(extract32(opcode, 22, 1)) {
                        case 0x1:       /* SVE prefetch (scalar + immediate) */
                            size = extract32(opcode, 13, 2);
                            break;
                        default:
                            size = 3;   /* 32-bit (TODO: How to set this for vector register?) */
                            break;
                    }
                    break;
                default:
                    switch(extract32(opcode, 13, 3)) {
                        case 0x6:       /* SVE prefetch (scalar + scalar) */
                            size = extract32(opcode, 23, 2);
                            break;
                        default:
                            if(extract32(opcode, 22, 1)==0x1 && extract32(opcode, 15, 1)==0x1) {    /* SVE load and broadcast*/
                                switch(extract32(opcode, 23, 2)) {
                                    case 0x0:
                                        size = extract32(opcode, 13, 2);
                                        break;
                                    case 0x1:
                                        size = (extract32(opcode, 13, 2)==0x0)? 2 : extract32(opcode, 13, 2);
                                        break;
                                    case 0x2:
                                        switch(extract32(opcode, 13, 2)) {
                                            case 0x0:
                                                size = 3;
                                                break;
                                            case 0x1:
                                                size = 2;
                                                break;
                                            case 0x2:
                                                size = 2;
                                                break;
                                            case 0x3:
                                                size = 3;
                                                break;
                                        }
                                        break;
                                    case 0x3:
                                        switch(extract32(opcode, 13, 2)) {
                                            case 0x0:
                                                size = 3;
                                                break;
                                            case 0x1:
                                                size = 2;
                                                break;
                                            case 0x2:
                                                size = 1;
                                                break;
                                            case 0x3:
                                                size = 3;
                                                break;
                                        }
                                        break;
                                } 
                            } else {        /* All others are 32 bit */
                                size = 3;
                            }
                            break;
                    }
                    break;
            }
            break;
        case 0x5:
            has_mem_access = true;  /* SVE Memory - Contiguous Load */
            is_store = false;       /* All Contiguous load instructions */
            g_assert(extract32(opcode, 25, 4) == 0x2);
            switch(extract32(opcode, 13, 3)) {
                case 0x0:
                    size = extract32(opcode, 23, 2);
                    break;
                case 0x2:
                case 0x3:
                case 0x5:
                    switch(extract32(opcode, 23, 2)) {
                        case 0x0:
                            size = extract32(opcode, 21, 2);
                            break;
                        case 0x1:
                            size = (extract32(opcode, 21, 2)==0x0)? 2 : extract32(opcode, 21, 2);
                            break;
                        case 0x2:
                            switch(extract32(opcode, 21, 2)) {
                                case 0x0:
                                    size = 3;
                                    break;
                                case 0x1:
                                    size = 2;
                                    break;
                                case 0x2:
                                    size = 2;
                                    break;
                                case 0x3:
                                    size = 3;
                                    break;
                            }
                            break;
                        case 0x3:
                            switch(extract32(opcode, 21, 2)) {
                                case 0x0:
                                    size = 3;
                                    break;
                                case 0x1:
                                    size = 2;
                                    break;
                                case 0x2:
                                    size = 1;
                                    break;
                                case 0x3:
                                    size = 3;
                                    break;
                            }
                    }
                    break;
                case 0x1:
                case 0x6:
                    size = extract32(opcode, 23, 2);
                    break;
                case 0x7:
                    switch(extract32(opcode, 21, 2)) {
                        case 0x0:
                            if(extract32(opcode, 20, 1) == 0x0)
                                size = extract32(opcode, 23, 2);
                            else {
                                size = 4;
                                printf("[Warn]:QFlex, loading quadwords, setting size_t = 4.\n");
                            }
                            break;
                        default:
                            g_assert(extract32(opcode, 20, 1) == 0x0); /* The other one is unallocated */
                            size = extract32(opcode, 23, 2);
                            break;
                    }
                    break;
                case 0x4:
                    switch(extract32(opcode, 21, 2)) {
                        case 0x0:
                        case 0x1:
                            size = extract32(opcode, 23, 2);
                            break;
                        default:
                            printf("[Error]:QFlex, Unallocated encoding.\n");
                            g_assert_not_reached();
                            break;
                    }
                    break;
            }
            break;
        case 0x6:
            has_mem_access = true;  /* SVE Memory - 64-bit Gather */
            size = 3;               /* 64-bit */
            is_store = false;       /* All Gather (Load) instructions */
            g_assert(extract32(opcode, 25, 4) == 0x2);
            break;
        case 0x7:
            has_mem_access = true;  /* Misc SVE store operations */
            is_store = true;
            g_assert(extract32(opcode, 25, 4) == 0x2);
            switch(extract32(opcode, 13, 3)) {
                case 0x0:
                case 0x2:
                    switch(extract32(opcode, 23, 2)) {
                        case 0x6:
                            size = 2;
                            break;
                        default:
                            if(extract32(opcode, 14, 1)==0x1)
                                size = extract32(opcode, 23, 2);
                            else {
                                size = 4;
                                printf("[Warn]:QFlex, loading quadwords, setting size_t = 4.\n");
                            }
                            break;
                    }
                    break;
                case 0x1:
                    if(extract32(opcode, 21, 1)==0x1) {
                        size = 4;
                        printf("[Warn]:QFlex, loading quadwords, setting size_t = 4.\n");
                    } else {
                        size = (extract32(opcode, 22, 1) == 0x0) ? 3 : 2;
                    }
                    break;
                case 0x3:
                case 0x4:
                case 0x6:
                case 0x7:
                    size = extract32(opcode, 23, 2);
                    break;
                case 0x5:
                    size = (extract32(opcode, 21, 2) == 0x3)? 2 : 3;
                    break;
            }
            break;
        default:
            has_mem_access = false;
            break;
    }

    *s = (struct mem_access) {.size = size,
        .is_vector = true,
        .is_load = !is_store,
        .is_store = is_store,
        .is_signed = false,         // TODO: Not handled for now because unused
        .is_pair = false,
        .is_atomic = is_atomic,
        .accesses = 1};             // TODO: Not handled for now because unused

    return has_mem_access;
}

bool
decode_armv8_mem_opcode(struct mem_access* s, uint32_t opcode)
{

    memset(s, 0, sizeof(struct mem_access));

    bool has_mem_access = false;

    switch (extract32(opcode, 25, 4)) {
    case 0x0:
        break;
    case 0x1: case 0x3: /* UNALLOCATED */
        // unallocated_encoding;
        break;
    case 0x2: /* SVE */
        has_mem_access = disas_sve(s, opcode);
        break;
    case 0x8: case 0x9: /* Data processing - immediate */
        break;
    case 0xa: case 0xb: /* Branch, exception generation and system opcodes */
        has_mem_access = disas_branch_sys(s, opcode);
        break;
    case 0x4:
    case 0x6:
    case 0xc:
    case 0xe:      /* Loads and stores */
        has_mem_access = disas_ldst(s, opcode);
        break;
    case 0x5:
    case 0xd:      /* Data processing - register */
        break;
    case 0x7:
    case 0xf:      /* Data processing - SIMD and floating point */
        break;
    default:
        g_assert_not_reached(); /* all 15 cases should be handled above */
    }

    if(!has_mem_access)
        printf("ERROR:QFlex, No memory access found for opcode: %x.\n", opcode);

    return has_mem_access;
}
