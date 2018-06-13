#include "flexus_proxy.h"
#include "api.h"

#include <stdio.h>
#include <errno.h>

void QFLEX_API_get_interface_hooks(QFLEX_API_Interface_Hooks_t* hooks) {
  hooks->cpu_read_register= cpu_read_register;
  hooks->cpu_write_register= cpu_write_register;
  hooks->mmu_logical_to_physical= mmu_logical_to_physical;
  hooks->cpu_get_program_counter= cpu_get_program_counter;
  hooks->cpu_get_address_space= cpu_get_address_space_flexus;                      // Changed name here
  hooks->cpu_proc_num= cpu_proc_num;
  hooks->cpu_pop_indexes= cpu_pop_indexes;
  hooks->QEMU_get_phys_memory= QEMU_get_phys_memory;
  hooks->QEMU_get_ethernet= QEMU_get_ethernet;
  hooks->QEMU_clear_exception= QEMU_clear_exception;
  hooks->QEMU_read_register= QEMU_read_register;
  hooks->QEMU_write_register= QEMU_write_register;
  hooks->QEMU_read_register_by_type= QEMU_read_register_by_type;
  hooks->QEMU_read_phys_memory= QEMU_read_phys_memory;
  hooks->QEMU_get_phys_mem= QEMU_get_phys_mem;
  hooks->QEMU_get_cpu_by_index= QEMU_get_cpu_by_index;
  hooks->QEMU_get_processor_number= QEMU_get_processor_number;
  hooks->QEMU_step_count= QEMU_step_count;
  hooks->QEMU_get_num_cpus= QEMU_get_num_cpus;
  hooks->QEMU_get_num_sockets= QEMU_get_num_sockets;
  hooks->QEMU_get_num_cores= QEMU_get_num_cores;
  hooks->QEMU_get_num_threads_per_core= QEMU_get_num_threads_per_core;
  hooks->QEMU_cpu_get_socket_id= QEMU_cpu_get_socket_id;
  hooks->QEMU_cpu_get_core_id= QEMU_cpu_get_core_id;
  hooks->QEMU_cpu_get_thread_id= QEMU_cpu_get_thread_id;
  hooks->QEMU_get_all_processors= QEMU_get_all_processors;
  hooks->QEMU_cpu_set_quantum= QEMU_cpu_set_quantum;
  hooks->QEMU_set_tick_frequency= QEMU_set_tick_frequency;
  hooks->QEMU_get_tick_frequency= QEMU_get_tick_frequency;
  hooks->QEMU_get_program_counter= QEMU_get_program_counter;
  hooks->QEMU_increment_debug_stat= QEMU_increment_debug_stat;
  hooks->QEMU_logical_to_physical= QEMU_logical_to_physical;
  hooks->QEMU_break_simulation= QEMU_break_simulation;
  hooks->QEMU_getSimulationTime=QEMU_getSimulationTime;
  hooks->QEMU_setSimulationTime=QEMU_setSimulationTime;
  hooks->QEMU_is_stopped=QEMU_is_stopped;
  hooks->QEMU_flush_all_caches= QEMU_flush_all_caches;
  hooks->QEMU_mem_op_is_data= QEMU_mem_op_is_data;
  hooks->QEMU_mem_op_is_write= QEMU_mem_op_is_write;
  hooks->QEMU_mem_op_is_read= QEMU_mem_op_is_read;
  hooks->QEMU_instruction_handle_interrupt = QEMU_instruction_handle_interrupt;
  hooks->QEMU_get_pending_exception = QEMU_get_pending_exception;
  hooks->QEMU_advance = QEMU_advance;
  hooks->QEMU_get_object = QEMU_get_object;
  hooks->QEMU_is_in_simulation = QEMU_is_in_simulation;
  hooks->QEMU_toggle_simulation = QEMU_toggle_simulation;
  hooks->QEMU_insert_callback= QEMU_insert_callback;
  hooks->QEMU_delete_callback= QEMU_delete_callback;
  hooks->QEMU_get_instruction_count = QEMU_get_instruction_count;
  hooks->QEMU_cpu_exec_proc = QEMU_cpu_exec_proc;
  hooks->QEMU_write_phys_memory= QEMU_write_phys_memory;
}

#include <stdlib.h>
#include <dlfcn.h>

SIMULATOR_INIT_PROC simulator_init = NULL;
SIMULATOR_PREPARE_PROC simulator_prepare = NULL;
SIMULATOR_DEINIT_PROC simulator_deinit = NULL;
SIMULATOR_START_PROC simulator_start = NULL;
SIMULATOR_BIND_QMP_PROC simulator_qmp = NULL;
SIMULATOR_BIND_CONFIG_PROC simulator_config = NULL;
struct simulator_obj{
  void* handle;
};

simulator_obj_t* simulator_load( const char* path ) {

    simulator_obj_t* module = (simulator_obj_t*)malloc(sizeof(simulator_obj_t));

  if( !module )
    return NULL;

  void* handle = dlopen( path, RTLD_LAZY );

  module->handle = handle;
  if( handle == NULL ) {
    printf("Can not load simulator from path %s\n", path);
    printf("error: %s\n", dlerror() );
    free ( module );
    return NULL;
  }

  simulator_init = (SIMULATOR_INIT_PROC)dlsym( handle, "qflex_init" );
  simulator_prepare = (SIMULATOR_PREPARE_PROC)dlsym( handle, "flexInit" );
  simulator_deinit = (SIMULATOR_DEINIT_PROC)dlsym( handle, "qflex_quit" );
  simulator_start = (SIMULATOR_START_PROC)dlsym( handle, "startTiming" );
  simulator_qmp = (SIMULATOR_BIND_QMP_PROC)dlsym( handle, "qmpcall" );
  simulator_config = (SIMULATOR_BIND_CONFIG_PROC)dlsym( handle, "setConfig" );

  if (simulator_init    == NULL ||
      simulator_prepare == NULL ||
      simulator_deinit  == NULL ||
      simulator_start   == NULL ||
      simulator_qmp     == NULL ||
      simulator_config  == NULL ){

      printf("simulator does not support all of APIs modules! - check you simulator for \"c\" functions wrappers\n");
      printf("error: %s\n", dlerror() );
      return NULL;
  }

  return module;
}

void simulator_unload( simulator_obj_t* obj ) {
  if( obj->handle != NULL )
    dlclose( obj->handle );

  free( obj );
}
