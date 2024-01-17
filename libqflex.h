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
 * */
struct vCPU 
{
    size_t index;
    CPUARMState* cpu;
    // Whatever this need at some point
};

typedef enum isa_regs_t {
    /**
     * Only type of instruction supported by Flexus to this date
     */
    ISAR_ID_AA64MMFR1
} 

typedef struct register_kwargs_t
{
    union 
    {
        size_t index;

        // struct {
        // /**
        //  * Location of register: coprocessor number and (crn,crm,opc1,opc2)
        //  * tuple. Any of crm, opc1 and opc2 may be CP_ANY to indicate a
        //  * 'wildcard' field -- any value of that field in the MRC/MCR insn
        //  * will be decoded to this register. The register read and write
        //  * callbacks will be passed an ARMCPRegInfo with the crn/crm/opc1/opc2
        //  * used by the program, so it is possible to register a wildcard and
        //  * then behave differently on read/write if necessary.
        //  * For 64 bit registers, only crm and opc1 are relevant; crn and opc2
        //  * must both be zero.
        //  * For AArch64-visible registers, opc0 is also used.
        //  * Since there are no "coprocessors" in AArch64, cp is purely used as a
        //  * way to distinguish (for KVM's benefit) guest-visible system registers
        //  * from demuxed ones provided to preserve the "no side effects on
        //  * KVM register read/write from QEMU" semantics. cp==0x13 is guest
        //  * visible (to match KVM's encoding); cp==0 will be converted to
        //  * cp==0x13 when the ARMCPRegInfo is registered, for convenience.
        //  */
        //     uint8_t cp;
        //     uint8_t crn; 
        //     uint8_t crm;
        //     uint8_t op0;
        //     uint8_t op1;
        //     uint8_t op2;
        // } cp15_regs;
        
        isa_regs_t isa_regs;
    };
    
};

// TRACE Spec'
// Reading register
void libqflex_read_register(struct vCPU, enum arm_register, register_kwargs_t*);
void libqflex_poll_irq();

// Reading memory
void libqflex_resolving_vaddr();


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