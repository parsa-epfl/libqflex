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
#include "qflex-api.h"

#include <stdio.h>
#include <errno.h>

void QFLEX_API_get_Interface_Hooks (QFLEX_TO_QEMU_API_t* hooks) {
  hooks->QEMU_clear_exception = QEMU_clear_exception;
  hooks->QEMU_read_register = QEMU_read_register;
  hooks->QEMU_read_unhashed_sysreg = QEMU_read_unhashed_sysreg;
  hooks->QEMU_write_register = QEMU_write_register;
  hooks->QEMU_read_sp_el = QEMU_read_sp_el;

  hooks->QEMU_has_pending_irq = QEMU_has_pending_irq;
  hooks->QEMU_read_sctlr = QEMU_read_sctlr;
  hooks->QEMU_read_pstate = QEMU_read_pstate;
  hooks->QEMU_cpu_has_work = QEMU_cpu_has_work;

  hooks->QEMU_read_phys_memory = QEMU_read_phys_memory;
  hooks->QEMU_get_cpu_by_index = QEMU_get_cpu_by_index;
  hooks->QEMU_get_cpu_index = QEMU_get_cpu_index;
  hooks->QEMU_step_count = QEMU_step_count;
  hooks->QEMU_get_num_sockets = QEMU_get_num_sockets;
  hooks->QEMU_get_num_cores = QEMU_get_num_cores;
  hooks->QEMU_get_num_threads_per_core = QEMU_get_num_threads_per_core;
  hooks->QEMU_get_all_cpus = QEMU_get_all_cpus;
  hooks->QEMU_cpu_set_quantum = QEMU_cpu_set_quantum;
  hooks->QEMU_set_tick_frequency = QEMU_set_tick_frequency;
  hooks->QEMU_get_tick_frequency = QEMU_get_tick_frequency;
  hooks->QEMU_get_program_counter = QEMU_get_program_counter;
  hooks->QEMU_increment_debug_stat = QEMU_increment_debug_stat;
  hooks->QEMU_logical_to_physical = QEMU_logical_to_physical;
  hooks->QEMU_quit_simulation = QEMU_quit_simulation;
  hooks->QEMU_getCyclesLeft = QEMU_getCyclesLeft;
  hooks->QEMU_mem_op_is_data= QEMU_mem_op_is_data;
  hooks->QEMU_mem_op_is_write= QEMU_mem_op_is_write;
  hooks->QEMU_mem_op_is_read= QEMU_mem_op_is_read;
  hooks->QEMU_instruction_handle_interrupt = QEMU_instruction_handle_interrupt;
  hooks->QEMU_get_object_by_name = QEMU_get_object_by_name;
  hooks->QEMU_is_in_simulation = QEMU_is_in_simulation;
  hooks->QEMU_toggle_simulation = QEMU_toggle_simulation;
  hooks->QEMU_get_instruction_count = QEMU_get_instruction_count;
  hooks->QEMU_cpu_execute = QEMU_cpu_execute;
  hooks->QEMU_disassemble = QEMU_disassemble;
  hooks->QEMU_dump_state = QEMU_dump_state;

  hooks->QEMU_flush_tb_cache = QEMU_flush_tb_cache;
  hooks->QEMU_cpu_set_quantum = QEMU_cpu_set_quantum;
  hooks->QEMU_increment_debug_stat = QEMU_increment_debug_stat;
}

QEMU_TO_QFLEX_CALLBACKS_t qflex_sim_callbacks;
void QEMU_API_set_Interface_Hooks (const QEMU_TO_QFLEX_CALLBACKS_t *hooks) {
//  qflex_sim_callbacks = hooks;
  qflex_sim_callbacks.sim_quit           = hooks->sim_quit;
  qflex_sim_callbacks.start_timing       = hooks->start_timing;
  qflex_sim_callbacks.qmp                = hooks->qmp;
  qflex_sim_callbacks.trace_mem          = hooks->trace_mem;
  qflex_sim_callbacks.trace_mem_dma      = hooks->trace_mem_dma;
  qflex_sim_callbacks.periodic           = hooks->periodic;
  qflex_sim_callbacks.magic_inst         = hooks->magic_inst;
  qflex_sim_callbacks.ethernet_frame     = hooks->ethernet_frame;
  qflex_sim_callbacks.xterm_break_string = hooks->xterm_break_string;
}

#include <stdlib.h>
#include <dlfcn.h>

QFLEX_INIT qflex_init_fn = NULL;

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

  qflex_init_fn  = (QFLEX_INIT) dlsym( handle, "qflex_sim_init" );

  if (qflex_init_fn  == NULL) {
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
