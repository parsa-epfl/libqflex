#include "qemu/osdep.h"
#include <glib/gtestutils.h>

#include "libqflex.h"

// ─────────────────────────────────────────────────────────────────────────────

/**
 * Assert that register index are in range before retrieval
 * @param idx Index to be accessed
 * @param min Lower register range
 * @param max Upper register range
 *
 */
static void
assert_index_in_range(size_t idx, size_t min, size_t max) {
    g_assert(min >= idx && idx < max);
}

/**
 * Return the content of an ISA register.
 * ! ISA does not ONLY contain 64bits register.
 * ! Beware with future implementation
 * @param cpu_wrapper Pointer to a CPU structure
 * @param isa ISA register type
 */
static uint64_t
dispatch_isa_register(vCPU_t* cpu_wrapper, isa_regs_t isa)
{
    switch (isa)
    {
    case ID_AA64MMFR1:
        return cpu_wrapper->arch->isar.id;
        break;

    default:
        g_assert_not_reached();
    };
}

// ─────────────────────────────────────────────────────────────────────────────

uint64_t
libqflex_read_register(
    vCPU_t* cpu_wrapper,
    enum arm_register_type reg_type,
    register_kwargs_t* read_args)
{

    size_t idx      = read_args->index;
    isa_regs_t isa  = read_args->isa_regs;

    switch (reg_type)
    {

    case GENERAL:
        assert_index_in_range(idx, 0, 32);
        return cpu_wrapper->arch->regs[idx];

    /**
     * aa32_vfp_dreg:
     * Return a pointer to the Dn register within env in 32-bit mode.
     */
    case FLOATING_POINT:
        /** Define a maximum sized vector register.
        * For 32-bit, this is a 128-bit NEON/AdvSIMD register.
        * For 64-bit, this is a 2048-bit SVE register.
        *
        * Note that the mapping between S, D, and Q views of the register bank
        * differs between AArch64 and AArch32.
        * In AArch32:
        *  Qn = regs[n].d[1]:regs[n].d[0]
        *  Dn = regs[n / 2].d[n & 1]
        *  Sn = regs[n / 4].d[n % 4 / 2],
        *       bits 31..0 for even n, and bits 63..32 for odd n
        *       (and regs[16] to regs[31] are inaccessible)
        * In AArch64:
        *  Zn = regs[n].d[*]
        *  Qn = regs[n].d[1]:regs[n].d[0]
        *  Dn = regs[n].d[0]
        *  Sn = regs[n].d[0] bits 31..0
        *  Hn = regs[n].d[0] bits 15..0
        *
        * This corresponds to the architecturally defined mapping between
        * the two execution states, and means we do not need to explicitly
        * map these registers when changing states.
        *
        * Align the data for use with TCG host vector operations.
        */
        assert_index_in_range(idx, 0, 64);
        return cpu_wrapper->arch->vfp.zregs[idx].d[0];

    case TTBR0:
        assert_index_in_range(idx, 0, 4);
        return cpu_wrapper->arch->cp15.ttbr0_el[idx];

    case TTBR1:
        assert_index_in_range(idx, 0, 4);
        return cpu_wrapper->arch->cp15.ttbr1_el[idx];

    case TCR:
        assert_index_in_range(idx, 0, 4);
        return cpu_wrapper->arch->cp15.tcr_el[idx];

    case ISA:
        return dispatch_isa_register(cpu_wrapper, isa);

    default:
        g_assert_not_reached();
    }

}