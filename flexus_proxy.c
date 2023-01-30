//  DO-NOT-REMOVE begin-copyright-block
// QFlex consists of several software components that are governed by various
// licensing terms, in addition to software that was developed internally.
// Anyone interested in using QFlex needs to fully understand and abide by the
// licenses governing all the software components.
// 
// ### Software developed externally (not by the QFlex group)
// 
//     * [NS-3] (https://www.gnu.org/copyleft/gpl.html)
//     * [QEMU] (http://wiki.qemu.org/License)
//     * [SimFlex] (http://parsa.epfl.ch/simflex/)
//     * [GNU PTH] (https://www.gnu.org/software/pth/)
// 
// ### Software developed internally (by the QFlex group)
// **QFlex License**
// 
// QFlex
// Copyright (c) 2020, Parallel Systems Architecture Lab, EPFL
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//     * Redistributions of source code must retain the above copyright notice,
//       this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright notice,
//       this list of conditions and the following disclaimer in the documentation
//       and/or other materials provided with the distribution.
//     * Neither the name of the Parallel Systems Architecture Laboratory, EPFL,
//       nor the names of its contributors may be used to endorse or promote
//       products derived from this software without specific prior written
//       permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE PARALLEL SYSTEMS ARCHITECTURE LABORATORY,
// EPFL BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//  DO-NOT-REMOVE end-copyright-block
#include "flexus_proxy.h"
#include "api.h"

#include <stdio.h>
#include <errno.h>

void QFLEX_API_get_Interface_Hooks (QFLEX_API_Interface_Hooks_t* hooks) {
  hooks->QEMU_get_ethernet= QEMU_get_ethernet;
  hooks->QEMU_clear_exception= QEMU_clear_exception;
  hooks->QEMU_read_register= QEMU_read_register;
  hooks->QEMU_read_unhashed_sysreg = QEMU_read_unhashed_sysreg;
  hooks->QEMU_write_register= QEMU_write_register;
  hooks->QEMU_read_sp_el = QEMU_read_sp_el;

  hooks->QEMU_read_fpcr = QEMU_read_fpcr;
  hooks->QEMU_read_fpsr = QEMU_read_fpsr;
  hooks->QEMU_read_exception = QEMU_read_exception;
  hooks->QEMU_get_pending_interrupt = QEMU_get_pending_interrupt;
  hooks->QEMU_read_sctlr = QEMU_read_sctlr;
  hooks->QEMU_read_tpidr = QEMU_read_tpidr;
  hooks->QEMU_read_pstate = QEMU_read_pstate;
  hooks->QEMU_read_hcr_el2 = QEMU_read_hcr_el2;
  hooks->QEMU_cpu_has_work = QEMU_cpu_has_work;

  hooks->QEMU_read_phys_memory= QEMU_read_phys_memory;
  hooks->QEMU_get_phys_mem= QEMU_get_phys_mem;
  hooks->QEMU_get_cpu_by_index= QEMU_get_cpu_by_index;
  hooks->QEMU_get_cpu_index= QEMU_get_cpu_index;
  hooks->QEMU_step_count= QEMU_step_count;
  hooks->QEMU_get_num_sockets= QEMU_get_num_sockets;
  hooks->QEMU_get_num_cores= QEMU_get_num_cores;
  hooks->QEMU_get_num_threads_per_core= QEMU_get_num_threads_per_core;
  hooks->QEMU_get_all_cpus= QEMU_get_all_cpus;
  hooks->QEMU_cpu_set_quantum= QEMU_cpu_set_quantum;
  hooks->QEMU_set_tick_frequency= QEMU_set_tick_frequency;
  hooks->QEMU_get_tick_frequency= QEMU_get_tick_frequency;
  hooks->QEMU_get_program_counter= QEMU_get_program_counter;
  hooks->QEMU_increment_debug_stat= QEMU_increment_debug_stat;
  hooks->QEMU_logical_to_physical= QEMU_logical_to_physical;
  hooks->QEMU_quit_simulation= QEMU_quit_simulation;
  hooks->QEMU_getCyclesLeft = QEMU_getCyclesLeft;
  hooks->QEMU_mem_op_is_data= QEMU_mem_op_is_data;
  hooks->QEMU_mem_op_is_write= QEMU_mem_op_is_write;
  hooks->QEMU_mem_op_is_read= QEMU_mem_op_is_read;
  hooks->QEMU_instruction_handle_interrupt = QEMU_instruction_handle_interrupt;
  hooks->QEMU_get_pending_exception = QEMU_get_pending_exception;
  hooks->QEMU_get_object_by_name = QEMU_get_object_by_name;
  hooks->QEMU_is_in_simulation = QEMU_is_in_simulation;
  hooks->QEMU_toggle_simulation = QEMU_toggle_simulation;
  hooks->QEMU_get_instruction_count = QEMU_get_instruction_count;
  hooks->QEMU_cpu_execute = QEMU_cpu_execute;
  hooks->QEMU_write_phys_memory= QEMU_write_phys_memory;
  hooks->QEMU_disassemble = QEMU_disassemble;
  hooks->QEMU_dump_state = QEMU_dump_state;
//  hooks->QEMU_get_mmu_state = QEMU_get_mmu_state;
}

#include <stdlib.h>
#include <dlfcn.h>

FLEXUS_SIM_DYNLIB_t flexus_dynlib_fns = {
    .qflex_sim_init  =  NULL,
    .qflex_sim_quit  =  NULL,
    .qflex_sim_start_timing =  NULL,
    .qflex_sim_qmp    =  NULL,
    .qflex_sim_callbacks = {
      .periodic = NULL,
      .trace_mem = NULL,
      .trace_mem_dma = NULL,
      .magic_inst = NULL,
      .ethernet_frame = NULL,
      .xterm_break_string = NULL,
    },
};
static void *handle = NULL;

// Dynamic library load
bool flexus_dynlib_load( const char* path ) {
  handle = dlopen( path, RTLD_LAZY );
  if( handle == NULL ) {
    printf("Can not load simulator dynamic library from path %s\n", path);
    printf("error: %s\n", dlerror() );
    free ( handle);
    return false;
  }

  flexus_dynlib_fns.qflex_sim_init  = (SIMULATOR_INIT_PROC)        dlsym( handle, "qflex_sim_init" );
  flexus_dynlib_fns.qflex_sim_quit  = (SIMULATOR_DEINIT_PROC)      dlsym( handle, "qflex_sim_quit" );
  flexus_dynlib_fns.qflex_sim_start_timing = (SIMULATOR_START_PROC)       dlsym( handle, "qflex_sim_start_timing" );
  flexus_dynlib_fns.qflex_sim_qmp    = (SIMULATOR_BIND_QMP_PROC)    dlsym( handle, "qflex_sim_qmp" );

  flexus_dynlib_fns.qflex_sim_callbacks.periodic = (SIMULATOR_PERIODIC_EVENT) dlsym( handle, "qflex_sim_callbacks_periodic");
  flexus_dynlib_fns.qflex_sim_callbacks.trace_mem = (SIMULATOR_TRACE_MEM) dlsym( handle, "qflex_sim_callbacks_trace_mem");
  flexus_dynlib_fns.qflex_sim_callbacks.trace_mem_dma = (SIMULATOR_TRACE_MEM_DMA) dlsym( handle, "qflex_sim_callbacks_trace_mem_dma");
  flexus_dynlib_fns.qflex_sim_callbacks.magic_inst = (SIMULATOR_MAGIC_INST) dlsym( handle, "qflex_sim_callbacks_magic_inst");
  flexus_dynlib_fns.qflex_sim_callbacks.ethernet_frame = (SIMULATOR_ETHERNET_FRAME) dlsym( handle, "qflex_sim_callbacks_ethernet_frame");
  flexus_dynlib_fns.qflex_sim_callbacks.xterm_break_string = (SIMULATOR_XTERM_BREAK_STRING) dlsym( handle, "qflex_sim_callbacks_xterm_break_string");

  if (flexus_dynlib_fns.qflex_sim_init  == NULL || 
    flexus_dynlib_fns.qflex_sim_quit  == NULL || 
    flexus_dynlib_fns.qflex_sim_start_timing == NULL || 
    flexus_dynlib_fns.qflex_sim_qmp    == NULL ||
    flexus_dynlib_fns.qflex_sim_callbacks.periodic == NULL ||
    flexus_dynlib_fns.qflex_sim_callbacks.trace_mem == NULL ||
    flexus_dynlib_fns.qflex_sim_callbacks.trace_mem_dma == NULL ||
    flexus_dynlib_fns.qflex_sim_callbacks.magic_inst == NULL ||
    flexus_dynlib_fns.qflex_sim_callbacks.ethernet_frame == NULL ||
    flexus_dynlib_fns.qflex_sim_callbacks.xterm_break_string == NULL
  ) {
      printf("simulator does not support all of APIs modules! - check you simulator for \"c\" functions wrappers\n");
      printf("error: %s\n", dlerror() );
      return false;
  }

  return true;
}

bool flexus_dynlib_unload(void) {
  if( handle != NULL )
    dlclose( handle );
  return true;
}
