#ifndef LIBQFLEX_H
#define LIBQFLEX_H

#include "target/arm/cpu.h"
#include "libqflex-legacy-api.h"

extern struct libqflex_state_t qemu_libqflex_state;



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


// ─────────────────────────────────────────────────────────────────────────────

void
libqflex_populate_vcpus(size_t n_vcpu);

/**
 * Read CPU Arch register.
 * ? Only implemented register can be accessed.
 *
 * @param size_t Virtual CPU index
 * @param arm_register_type An ARM specific register
 * @param register_kwargs_t A structure holding misc
 *                          information to retrieve a register
 *
 * @return A 64bits register content
 */

uint64_t
libqflex_read_register(
    size_t,
    register_type_t,
    register_kwargs_t);

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