#ifndef LIBQFLEX_H
#define LIBQFLEX_H

#include "target/arm/cpu.h"

enum arm_register_type
{
    GENERAL, // Regs for A64 mode.
    FLOATING_POINT,

    // PC,     // Program counter
    // PSTATE, // PSTATE isn't an architectural register for ARMv8
    // SYSREG, //maybe
    TTBR0,  // MMU translation table base 0
    TTBR1,  // MMU translation table base 1

    // SCTLR,  //system control register
    // SP,     // AArch64 banked stack pointers
    TCR,
    ISA,
    // TPID,   // User RW Thread register
    // VECTOR  // VFP coprocessor register

};

/**
 * This is a vCPU wrapper.
 * This wrap the famous CPUARMState from QEMU
 * @link https://gitlab.com/qemu-project/qemu/-/blob/master/target/arm/cpu.h
 *
 * Bryan Perdrizat
 *      The architectural state of a CPU is only available when
 *      compiling against a specific target. This is why meson.build add
 *      libqflex to `arm_ss`. The tradeoff is to access CPU arch. without
 *      requiring any additional invasive function.
 *
 *      tl:dr; ARMCPU, CPUArchState, ARMCPUClass because this file
 *      is compiled with the rest of the ARM target.
 **/
typedef struct
{
    // vCPU index (informative)
    size_t index;

    // High-level logical access to a vCPU,
    // eg: vCPU state, memory allocation,
    CPUState* state;
    // ARM Specific vCPU features,
    // eg: isa register, memory region, extension activated
    ARMCPU* cpu;
    // Architecture state of an ARM Core
    // eg: generic register, pc, pstate, spsr, cpsr, ...
    CPUArchState* env;

    // -*- Whatever is needed -*-
} vCPU_t;

typedef enum {
    /**
     * Only type of instruction supported by Flexus to this date
     */
    ID_AA64MMFR1,
} isa_regs_t;

/**
 * Structure holding misc informations for accessing register.
 */
typedef struct
{
    union
    {
        // Register index to access, ex. r01, r15, x19
        size_t index;

        // ISA specific register type if needed.
        isa_regs_t isa_regs;
    };

} register_kwargs_t;

// ─────────────────────────────────────────────────────────────────────────────

/**
 * Handle to access all vCPU structure and informations
 */
extern vCPU_t* libqflex_vcpus;

// ─────────────────────────────────────────────────────────────────────────────

// TRACE Spec'
// Reading register

/**
 * Read CPU Arch register.
 * ? Only implemented register can be accessed.
 *
 * @param vCPU_t Wrapper of a virtual CPU.
 * @param arm_register_type An ARM specific register
 * @param register_kwargs_t A structure holding misc
 *                          informatio to retrieve a register
 *
 * @return A 64bits register content
 */
uint64_t libqflex_read_register(
    vCPU_t*,
    enum arm_register_type,
    register_kwargs_t*);

void libqflex_poll_irq(void);

// Reading memory
void libqflex_resolving_vaddr(void);



/**
 * USED IN FLEXUS
 *
 * QEMU_read_unhashed_sysreg
 * QEMU_read_pstate
 * QEMU_has_pending_irq
 * QEMU_read_sp_el
 * QEMU_cpu_has_work
 * QEMU_read_sctlr
 * QEMU_read_phys_memory
 * QEMU_get_cpu_by_index
 * QEMU_get_num_cores
 * QEMU_get_program_counter
 * QEMU_logical_to_physical
 * QEMU_mem_op_is_data
 * QEMU_mem_op_is_write
 * QEMU_get_object_by_name
 * QEMU_cpu_execute
 * QEMU_flush_tb_cache
 * QEMU_getCyclesLeft
 * QEMU_quit_simulation
 * QEMU_get_instruction_count
 *
 * ! certainely usless
 * QEMU_dump_state
 * QEMU_disassemble
 * QEMU_clear_exception
 * QEMU_write_register
 * QEMU_step_count
 * QEMU_get_num_sockets
 * QEMU_get_num_threads_per_core
 * QEMU_get_all_cpus
 * QEMU_set_tick_frequency
 * QEMU_get_tick_frequency
 * QEMU_increment_debug_stat
 * QEMU_instruction_handle_interrupt
 * QEMU_toggle_simulation
 * QEMU_is_in_simulation
 * QEMU_increment_instruction_count
 * QEMU_get_total_instruction_count
 * QEMU_cpu_set_quantum
 */

#endif