
// TRACE Spec'
// Reading register
void libqflex_read_register();
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
