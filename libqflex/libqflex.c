#include "qemu/osdep.h"

#include "hw/core/tcg-cpu-ops.h"
#include "sysemu/cpu-timers.h"
#include "include/qemu/seqlock.h"
#include "include/sysemu/cpu-timers-internal.h"
#include "accel/tcg/tcg-accel-ops-icount.h"
#include "qemu/log.h"
#include "qapi/qapi-commands-misc.h"
#include "qapi/qapi-commands-control.h"
#include "sysemu/runstate.h"
#include "include/disas/disas.h"

#include "libqflex.h"
#include "libqflex-module.h"
#include "libqflex-legacy-api.h"

#include "target/arm/cpregs.h" // Need to be last
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
    g_assert(min <= idx && idx <= max);
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

        libqflex_vcpus[i].cc = CPU_GET_CLASS(libqflex_vcpus[i].state);
        g_assert(libqflex_vcpus[i].cc != NULL);

        libqflex_vcpus[i].env = cpu_env(libqflex_vcpus[i].state);
        g_assert(libqflex_vcpus[i].env != NULL);

        libqflex_vcpus[i].cpu = ARM_CPU(libqflex_vcpus[i].state);
        g_assert(libqflex_vcpus[i].cpu != NULL);
    }

    qemu_log("> [Libqflex] Populated %zu cpu(s)\n", n_vcpu);
}

uint64_t
libqflex_read_register(size_t cpu_index, register_type_t reg_type, size_t idx)
{

    vCPU_t* cpu_wrapper = lookup_vcpu(cpu_index);

    switch (reg_type)
    {

    case GENERAL:
        assert_index_in_range(idx, 0, 31);
        return cpu_wrapper->env->xregs[idx];

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
        assert_index_in_range(idx, 0, 63);
        return cpu_wrapper->env->vfp.zregs[idx].d[0];
        break;

    case PC:
        return cpu_wrapper->env->pc;

    case PSTATE:
        return cpu_wrapper->env->pstate;

    case SCTLR:
        assert_index_in_range(idx, 1, 3);
        return cpu_wrapper->env->cp15.sctlr_el[idx];
        break;

    case TTBR0:
        assert_index_in_range(idx, 1, 3);
        return cpu_wrapper->env->cp15.ttbr0_el[idx];

    case TTBR1:
        assert_index_in_range(idx, 1, 2);
        return cpu_wrapper->env->cp15.ttbr1_el[idx];

    case TCR:
        assert_index_in_range(idx, 1, 3);
        return cpu_wrapper->env->cp15.tcr_el[idx];

    case ID_AA64MMFR0:
        return cpu_wrapper->cpu->isar.id_aa64mmfr0;
        break;

    default:
        g_assert_not_reached();
    };

}


uint64_t
libqflex_read_sysreg(size_t cpu_index, uint8_t op0, uint8_t op1, uint8_t op2, uint8_t crn, uint8_t crm)
{
    vCPU_t* cpu_wrapper = lookup_vcpu(cpu_index);

    ARMCPRegInfo const * ri = get_arm_cp_reginfo(
        cpu_wrapper->cpu->cp_regs,
        ENCODE_AA64_CP_REG(CP_REG_ARM64_SYSREG_CP, crn, crm, op0, op1, op2));

    if (!ri) {
        // Unknown register; this might be a guest error or a QEMU
        // unimplemented feature.
        qemu_log("ERROR: read access to unsupported AArch64 "
                      "system register op0:%d op1:%d crn:%d crm:%d op2:%d\n",
                      op0, op1, crn, crm, op2);
        //return 0;
    }
    // Check access permissions
    if (!cp_access_ok(arm_current_el(cpu_wrapper->env), ri, true)) {
        qemu_log("ERROR: access to sysreg with wrong permissions");
        g_assert_not_reached();
    }

    if (ri && !(ri->type & ARM_CP_NO_RAW))
    {
        return read_raw_cp_reg(cpu_wrapper->env, ri);
    }

    // Msutherl: do it the slow way by linear searching if previous encoding didn't work
    for (size_t i = 0; i < cpu_wrapper->cpu->cpreg_array_len; i++)
    {

        uint32_t regidx = kvm_to_cpreg_id(cpu_wrapper->cpu->cpreg_indexes[i]);

        ri = get_arm_cp_reginfo(cpu_wrapper->cpu->cp_regs, regidx);

        if (ri->opc0 == op0 &&
            ri->opc1 == op1 &&
            ri->opc2 == op2 &&
            ri->crn == crn &&
            ri->crm == crm &&
            !(ri->type & ARM_CP_NO_RAW)) {

            return read_raw_cp_reg(cpu_wrapper->env, ri);
        }
    }

    qemu_log("ERROR: QEMU did not recognize sysreg case");
    g_assert_not_reached();
    return 0;
}

size_t
libqflex_get_nb_cores(void)
{
    return qemu_libqflex_state.n_vcpus;
}

bool
libqflex_is_core_busy(size_t cpu_index)
{
    vCPU_t* cpu_wrapper = lookup_vcpu(cpu_index);
    return !cpu_wrapper->state->halted;
}

physical_address_t
libqflex_translate_va2pa(size_t cpu_index, logical_address_t va)
{
    MemTxAttrs attrs;
    vCPU_t* cpu_wrapper = lookup_vcpu(cpu_index);

    hwaddr pa = arm_cpu_get_phys_page_attrs_debug(cpu_wrapper->state, va, &attrs);

    // Return the error if there is one, otherwise cast the returned address
    return (pa == -1) ? -1 : (physical_address_t)pa;
}

logical_address_t
libqflex_get_pc(size_t cpu_index)
{
    vCPU_t* cpu_wrapper = lookup_vcpu(cpu_index);
    return cpu_wrapper->env->pc;
}

bool
libqflex_has_interrupt(size_t cpu_index)
{

    vCPU_t* cpu_wrapper = lookup_vcpu(cpu_index);
    return cpu_wrapper
        ->cc
        ->tcg_ops
            ->cpu_check_interrupt(
                cpu_wrapper->state,
                cpu_wrapper->state->interrupt_request);
}


void
libqflex_tick(void)
{
    g_assert(qemu_libqflex_state.is_configured);
    g_assert(qemu_libqflex_state.is_running);
    g_assert(qemu_libqflex_state.mode == MODE_TIMING);

    // proceed one clock cycle
    seqlock_write_lock(&timers_state.vm_clock_seqlock, &timers_state.vm_clock_lock);
    qatomic_set_i64(&timers_state.qemu_icount, timers_state.qemu_icount + 1);
    seqlock_write_unlock(&timers_state.vm_clock_seqlock, &timers_state.vm_clock_lock);
    // fire qemu timers to generate guest timer interrupts
    icount_account_warp_timer();
    icount_handle_deadline();

}

uint64_t
libqflex_advance(size_t cpu_index, bool trigger_count)
{
    vCPU_t* cpu_wrapper = lookup_vcpu(cpu_index);
    if (trigger_count) qemu_libqflex_state.cycles--;

    return libqflex_step(cpu_wrapper->state);
}

char* libqflex_disas(size_t cpu_index, uint64_t addr, size_t size)
{
    vCPU_t* cpu_wrapper = lookup_vcpu(cpu_index);
    return plugin_disas(cpu_wrapper->state, addr, size);
}

void
libqflex_stop(char const * const msg)
{
    Error* err = NULL;
    qemu_log("> [Libqflex] Stopping: %s\n", msg);

    qmp_stop(&err);

//    if (qemu_libqflex_state.is_running) {

        //if (qflex_state.update && (qflex_state.cycles <= 0))
        //    qflex_state.update++;
        //else
        //qmp_quit(errp);
  //  }
    qemu_libqflex_state.is_running = false;
    qmp_quit(&err);
}

void
libqflex_read_main_memory(uint8_t* buffer, physical_address_t pa, size_t bytes)
{

    assert_index_in_range(bytes, 0, 15);

    if ((int64_t)(pa) < 0) {
        g_assert_not_reached();
        //memset(buffer, -1, bytes);
    }

    cpu_physical_memory_read(pa, buffer, bytes);
}

void
libqflex_save_ckpt(char const * const dirname)
{
    int saved_vm_running = runstate_is_running();
    vm_stop(RUN_STATE_SAVE_VM);

    g_mkdir_with_parents(dirname, 0700);
    flexus_api.qmp(QMP_FLEXUS_DOSAVE, dirname);

    if (saved_vm_running)
        vm_start();
}

void
libqflex_load_ckpt(char const * const dirname)
{
    int saved_vm_running = runstate_is_running();
    vm_stop(RUN_STATE_SAVE_VM);

    flexus_api.qmp(QMP_FLEXUS_DOLOAD, dirname);

    if (saved_vm_running)
        vm_start();
}
// int
// libqflex_get_el(size_t cpu_index)
// {

//     vCPU_t* cpu_wrapper = lookup_vcpu(cpu_index);

//     return arm_current_el(cpu_wrapper->env);

// }
