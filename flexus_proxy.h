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
#ifndef __LIBQEMUFLEX_FLEXUS_PROXY_HPP__
#define __LIBQEMUFLEX_FLEXUS_PROXY_HPP__
#include "api.h"

bool flexus_dynlib_load( const char* path );
bool flexus_dynlib_unload( void );

typedef void (*SIMULATOR_INIT_PROC)(QFLEX_API_Interface_Hooks_t*, int, const char*);
typedef void (*SIMULATOR_PREPARE_PROC)(void);
typedef void (*SIMULATOR_DEINIT_PROC)(void);
typedef void (*SIMULATOR_START_PROC)(void);
typedef void (*SIMULATOR_BIND_QMP_PROC)(qmp_flexus_cmd_t, const char*);
typedef void (*SIMULATOR_BIND_CONFIG_PROC)(const char*);

typedef void (*SIMULATOR_BIND_CONFIG_PROC)(const char*);


// Callbacks for trace mode, see `api.h` at end of file
typedef void (*SIMULATOR_TRACE_MEM)(int, memory_transaction_t*);
typedef void (*SIMULATOR_TRACE_MEM_DMA)(memory_transaction_t*);
typedef void (*SIMULATOR_PERIODIC_EVENT)(void);
typedef void (*SIMULATOR_MAGIC_INST)(int, long long);
typedef void (*SIMULATOR_ETHERNET_FRAME)(int32_t, int32_t, long long);
typedef void (*SIMULATOR_XTERM_BREAK_STRING)(char *);
	
typedef struct FLEXUS_SIM_DYNLIB_CALLBACK_t {
	SIMULATOR_TRACE_MEM trace_mem;
	SIMULATOR_TRACE_MEM_DMA trace_mem_dma;
	SIMULATOR_PERIODIC_EVENT periodic;
	SIMULATOR_MAGIC_INST magic_inst;
  	SIMULATOR_ETHERNET_FRAME ethernet_frame;
  	SIMULATOR_XTERM_BREAK_STRING xterm_break_string;
} FLEXUS_SIM_DYNLIB_CALLBACK_t;

typedef struct FLEXUS_SIM_DYNLIB_t {
	SIMULATOR_INIT_PROC        qflex_sim_init;
	SIMULATOR_DEINIT_PROC      qflex_sim_quit;
	SIMULATOR_START_PROC       qflex_sim_start_timing;
	SIMULATOR_BIND_QMP_PROC    qflex_sim_qmp;
	FLEXUS_SIM_DYNLIB_CALLBACK_t qflex_sim_callbacks;
} FLEXUS_SIM_DYNLIB_t;

extern FLEXUS_SIM_DYNLIB_t flexus_dynlib_fns;

#endif /* __LIBQEMUFLEX_FLEXUS_PROXY_HPP__ */
