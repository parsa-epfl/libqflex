#include "qemu/osdep.h"

#include "qemu/log.h"

#include "libqflex.h"
#include "libqflex-module.h"
#include "libqflex-legacy-api.h"

// ─────────────────────────────────────────────────────────────────────────────

vCPU_t* libqflex_vcpus = NULL;

// ─────────────────────────────────────────────────────────────────────────────

/**
 * Assert that register index are in range before retrieval
 * @param idx Index to be accessed
 * @param min Lower register range
 * @param max Upper register range
 *
 */
static inline void
assert_index_in_range(size_t idx, size_t min, size_t max) {
    g_assert(min <= idx && idx < max);
}

/**
 * Assert that we are always looking for a core
 * that is actually emulated by QEMU.
 * @param idx A CPU/core index to access
 */
static inline vCPU_t*
lookup_vcpu(size_t idx)
{
    assert_index_in_range(idx, 0, qemu_libqflex_state.n_vcpus);
    return &libqflex_vcpus[idx];
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
        return cpu_wrapper->cpu->isar.id_aa64mmfr1;
        break;

    default:
        g_assert_not_reached();
    };
}

/**
 * Retrieve pointer to CPU State and CPU Arch State from the global memory.
 * This avoid to retrieve and loop over cpu for every register access.
 *
 * Bryan Perdrizat
 *      Here we prefer accessing vCPU though their index for simplicity,
 *      despite that there exists other access method, like CPU_FOREACH(),
 *      and that it rely on the -smp parsing which may set thread/socket instead
 *      of cpus.
 *
 * @param n_vcpu Number of vCPU to retrieve.
 */
void
libqflex_populate_vcpus(size_t n_vcpu)
{
    //! DON'T FORGET TO FREE THE ARRAY ALLOCATION
    //! NOT THE CPU ITSELF
    libqflex_vcpus = g_new0(vCPU_t, n_vcpu);

    for(size_t i = 0; i < n_vcpu; i++)
    {
        libqflex_vcpus[i].index = i;

        libqflex_vcpus[i].state = qemu_get_cpu(i);
        g_assert(libqflex_vcpus[i].state != NULL);

        libqflex_vcpus[i].env = cpu_env(libqflex_vcpus[i].state);
        g_assert(libqflex_vcpus[i].env != NULL);

        libqflex_vcpus[i].cpu = ARM_CPU(libqflex_vcpus[i].state);
        g_assert(libqflex_vcpus[i].cpu != NULL);
    }

    qemu_log("> [Libqflex] Populated %zu cpu(s)\n", n_vcpu);
}

uint64_t
libqflex_read_register(size_t cpu_index, register_type_t reg_type, register_kwargs_t read_args)
{

    vCPU_t* cpu_wrapper = lookup_vcpu(cpu_index);

    size_t idx      = read_args.index;
    isa_regs_t isa  = read_args.isa_regs;

    switch (reg_type)
    {

    case GENERAL:
        assert_index_in_range(idx, 0, 32);
        return cpu_wrapper->env->regs[idx];

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
        return cpu_wrapper->env->vfp.zregs[idx].d[0];

    case TTBR0:
        assert_index_in_range(idx, 0, 4);
        return cpu_wrapper->env->cp15.ttbr0_el[idx];

    case TTBR1:
        assert_index_in_range(idx, 0, 4);
        return cpu_wrapper->env->cp15.ttbr1_el[idx];

    case TCR:
        assert_index_in_range(idx, 0, 4);
        return cpu_wrapper->env->cp15.tcr_el[idx];

    case ISA:
        return dispatch_isa_register(cpu_wrapper, isa);

    default:
        g_assert_not_reached();
    };

}
