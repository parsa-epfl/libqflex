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
#ifdef __cplusplus
extern "C" {
#endif

#include "qemu/osdep.h"
#include "qemu/config-file.h"
#include "hw/core/cpu.h"
#include "sysemu/cpus.h"
#include "qemu/main-loop.h"
#include "qapi/error.h"
#include "qapi/qapi-commands-control.h"
#include "hw/boards.h"
#include "qemu/option.h"

#include "qflex-api.h"
#include "qflex/qflex-arch.h"
#include "qflex/qflex.h"

static int64_t cyclesLeft = -1;
static bool timing;
static int debugStats[ALL_DEBUG_TYPE] = {0};
static conf_object_t *qemu_cpus = NULL;
static bool qemu_objects_initialized;

#ifdef CONFIG_QUANTUM
extern uint64_t quantum_value;
#endif

static int flexus_is_simulating;

uint64_t QEMU_read_unhashed_sysreg(conf_object_t *cpu, uint8_t op0, uint8_t op1, uint8_t op2, uint8_t crn, uint8_t crm) {
    CPUState *cs = qemu_get_cpu(((CPUState*)cpu->object)->cpu_index);
    return QFLEX_GET_ARCH(sysreg)(cs, op0, op1, op2, crn, crm);
}

void QEMU_dump_state(conf_object_t* cpu, char** buf_ptr) {
    CPUState *cs = qemu_get_cpu(((CPUState*)cpu->object)->cpu_index);
    qflex_dump_archstate_log(cs, buf_ptr);
}

void QEMU_disassemble(conf_object_t* cpu, uint64_t addr, char **buf_ptr){
    CPUState *cs = qemu_get_cpu(((CPUState*)cpu->object)->cpu_index);
    QFLEX_GET_ARCH(log_inst_buffer)(cs, addr, buf_ptr);
}

int QEMU_clear_exception(void){
    assert(false);
}

void QEMU_write_register(conf_object_t *cpu, arm_register_t reg_type, int reg_index, uint64_t value)
{
  assert(reg_type == kGENERAL);
  CPUState *cs = qemu_get_cpu(((CPUState*)cpu->object)->cpu_index);
  QFLEX_SET_ARCH(reg)(cs, reg_index, value);
  assert(QFLEX_GET_ARCH(reg)(cs, reg_index) == value);
}

uint64_t QEMU_read_register(conf_object_t *cpu, arm_register_t reg_type, int reg_index, int el) {
  CPUState *cs = qemu_get_cpu(((CPUState *)cpu->object)->cpu_index);
  switch (reg_type) {
  case kGENERAL:
    return QFLEX_GET_ARCH(reg)(cs, reg_index);
  case kFLOATING_POINT:
    assert(reg_index <= 31 && reg_index >= 0);
    return QFLEX_GET_ARCH(vfp_reg)(cs, reg_index);
  case kMMU_TCR: 
    return QFLEX_GET_ARCH(tcr)(cs, el);
  case kMMU_SCTLR:
    return QFLEX_GET_ARCH(sctlr)(cs, el);
  case kMMU_TTBR0:
    return QFLEX_GET_ARCH(ttbr0)(cs, el);
  case kMMU_TTBR1:
    return QFLEX_GET_ARCH(ttbr1)(cs, el);
  case kMMU_ID_AA64MMFR0:
    return QFLEX_GET_ARCH(isa_reg)(cs, ISAR_ID_AA64MMFR1);
  default:
    fprintf(stderr,
            "ERROR case triggered in readReg. reg_idx: %d, reg_type: %d\n, el: %d",
            reg_index, reg_type, el);
    assert(false);
    break;
  }

}

uint32_t QEMU_read_pstate(conf_object_t *cpu) {
  CPUState *cs = qemu_get_cpu(((CPUState*)cpu->object)->cpu_index);
  return QFLEX_GET_ARCH(pstate)(cs);
}

bool QEMU_has_pending_irq(conf_object_t *cpu) {
  CPUState *cs = qemu_get_cpu(((CPUState*)cpu->object)->cpu_index);
  return QFLEX_GET_ARCH(has_irq)(cs);
}

uint64_t QEMU_read_sp_el(uint8_t id, conf_object_t *cpu){
  CPUState *cs = qemu_get_cpu(((CPUState *)cpu->object)->cpu_index);
  return QFLEX_GET_ARCH(sp_el)(cs, id);
}

bool QEMU_cpu_has_work(conf_object_t *cpu){
  CPUState *cs = qemu_get_cpu(((CPUState *)cpu->object)->cpu_index);
  return !cs->halted;
}

uint64_t QEMU_read_sctlr(uint8_t id, conf_object_t *cpu) {
    CPUState *cs = qemu_get_cpu(((CPUState *)cpu->object)->cpu_index);
    return QFLEX_GET_ARCH(sctlr)(cs, id);
}

void QEMU_read_phys_memory(uint8_t *buf, physical_address_t pa, int bytes) {
    assert(0 <= bytes && bytes <= 16);
    cpu_physical_memory_read(pa, buf, bytes);
}

conf_object_t *QEMU_get_cpu_by_index(int index) {
    if (qemu_objects_initialized)
        return &qemu_cpus[index];
    assert(false);
}

int QEMU_get_cpu_index(conf_object_t *cpu) {
    CPUState *cs = (CPUState *) cpu->object;
    return cs->cpu_index;
}

uint64_t QEMU_step_count(conf_object_t *cpu) {
  return QEMU_get_instruction_count(QEMU_get_cpu_index(cpu), BOTH_INSTR);
}

int QEMU_get_num_sockets(void) {
  unsigned nsockets = 1;
  QemuOpts * opts = (QemuOpts*)qemu_opts_find( qemu_find_opts("smp-opts"), NULL );
  if( opts != NULL )
    nsockets = qemu_opt_get_number(opts, "sockets", 0);
  return  nsockets > 0 ? nsockets : 1;
}

int QEMU_get_num_cores(void) {
  return current_machine->smp.cores;
}

int QEMU_get_num_threads_per_core(void) {
  return current_machine->smp.threads;
}

// return the id of the socket of the processor
//int QEMU_cpu_get_socket_id(conf_object_t *cpu) {
//  int threads_per_core = QEMU_get_num_threads_per_core();
//  int cores_per_socket = QEMU_get_num_cores();
//  int cpu_id = QEMU_get_processor_number(cpu);
//  return cpu_id / (cores_per_socket * threads_per_core);
//}

conf_object_t * QEMU_get_all_cpus(void) {
    if (qemu_objects_initialized)
        return qemu_cpus;

    assert(false);
}

int QEMU_set_tick_frequency(conf_object_t *cpu, double tick_freq) 
{
	return 42;
}

double QEMU_get_tick_frequency(conf_object_t *cpu){
	return 3.14;
}

uint64_t QEMU_get_program_counter(conf_object_t * cpu) {
    CPUState *cs = qemu_get_cpu(((CPUState *)cpu->object)->cpu_index);
    return QFLEX_GET_ARCH(pc)(cs);
}

void QEMU_increment_debug_stat(int val) {
    debugStats[val]++;
}

// This is GUEST PA, not HOST PA
physical_address_t QEMU_logical_to_physical(conf_object_t *cpu, 
		data_or_instr_t fetch, logical_address_t va) 
{
    int access_type = (fetch == QEMU_DI_Instruction ? INST_FETCH : DATA_LOAD);
    CPUState *cs = qemu_get_cpu(((CPUState *)cpu->object)->cpu_index);
    return gva_to_gpa(cs, va, access_type);
}

//------------------------Should be fairly simple here-------------------------

//[???] I assume these are treated as booleans were 1 = true, 0 = false
int QEMU_mem_op_is_data(generic_transaction_t *mop)
{
	//[???]not sure on this one
	//possibly
	//if it is read or write. Might also need cache?
	if((mop->type == QEMU_Trans_Store) || (mop->type == QEMU_Trans_Load)){
		return 1;
	}else{
		return 0;
	}
}

int QEMU_mem_op_is_write(generic_transaction_t *mop)
{
	//[???]Assuming Store == write.
	if(mop->type == QEMU_Trans_Store){
		return 1;
	}else{
		return 0;
	}
}

int QEMU_mem_op_is_read(generic_transaction_t *mop)
{
	//[???]Assuming load == read.
	if(mop->type == QEMU_Trans_Load){
		return 1;
	}else{
		return 0;
	}
}

instruction_error_t QEMU_instruction_handle_interrupt(conf_object_t *cpu, pseudo_exceptions_t pendingInterrupt) {
	return QEMU_IE_OK;
}
 
conf_object_t * QEMU_get_object_by_name(const char *name) {

    if (!qemu_objects_initialized)
        assert(false);

    unsigned int i;
    for(i = 0; i < current_machine->smp.cores; i++) {
        if(strcmp(qemu_cpus[i].name, name) == 0){
            return &(qemu_cpus[i]);
        }
	  }
    return NULL;
}

int QEMU_cpu_execute (conf_object_t *cpu, bool count_time) {
  
  assert(cpu->type == QEMU_CPUState);

  int ret = 0;
  CPUState *cs = qemu_get_cpu(((CPUState *)cpu->object)->cpu_index);
  ret = qflex_singlestep(cs);
  if (count_time) {
      cyclesLeft--;
  }
  return ret;
}


int QEMU_is_in_simulation(void) {
  return flexus_is_simulating;
}

void QEMU_toggle_simulation(int enable) {
  if( enable != QEMU_is_in_simulation() ) {
    flexus_is_simulating = enable;
    QEMU_flush_tb_cache();
  }
}

void QEMU_flush_tb_cache(void) {
  qflex_tb_flush();
}

uint64_t QEMU_total_instruction_count = 0;
uint64_t *QEMU_instruction_counts = NULL;
uint64_t *QEMU_instruction_counts_user = NULL;
uint64_t *QEMU_instruction_counts_OS = NULL;

static void qflex_api_populate_qemu_cpus(void)
{
    int ncpus = current_machine->smp.cores;
    qemu_cpus = malloc(sizeof(conf_object_t)*ncpus);

    int i = 0;
    for ( ; i < ncpus; i++) {
        qemu_cpus[i].type = QEMU_CPUState;

        int needed = snprintf(NULL, 0, "cpu%d", i ) + 1;
        qemu_cpus[i].name = malloc(needed);
        snprintf(qemu_cpus[i].name, needed, "cpu%d", i);
        qemu_cpus[i].object = qemu_get_cpu(i);
        if(! qemu_cpus[i].object){
            assert(false);
        }

    }
}

static void qflex_api_init_counts(void) {
  int num_cpus = current_machine->smp.cores;
  QEMU_instruction_counts= (uint64_t*)malloc(num_cpus*sizeof(uint64_t));

  QEMU_instruction_counts_user = (uint64_t*)malloc(num_cpus*sizeof(uint64_t));
  QEMU_instruction_counts_OS = (uint64_t*)malloc(num_cpus*sizeof(uint64_t));

  QEMU_total_instruction_count = 0;
  int i = 0;
  for( ; i < num_cpus; i++ ){
    QEMU_instruction_counts[i] = 0;

    QEMU_instruction_counts_user[i] = 0;
    QEMU_instruction_counts_OS[i] = 0;
  }
}

static QEMUTimer *exit_timer;
static void exit_timer_cb(void *opaque) {
  qmp_quit(NULL);
}

void qflex_api_init(bool timing_mode, uint64_t sim_cycles) {
  if (qemu_objects_initialized)
      assert(false);

  timing = timing_mode;
  cyclesLeft = sim_cycles;

  qflex_api_init_counts();
  qflex_api_populate_qemu_cpus();

  if (sim_cycles && qflexState.config.sim_cycles_cpuFirst == -1) {
    exit_timer = timer_new_ns(QEMU_CLOCK_VIRTUAL, exit_timer_cb, NULL);
    timer_mod(exit_timer, qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL) + sim_cycles);
  }

  qemu_objects_initialized = true;
}

static void qflex_deinitialize_counts(void) {
  free(QEMU_instruction_counts);
  free(QEMU_instruction_counts_user);
  free(QEMU_instruction_counts_OS);
}

void qflex_api_shutdown(void) {
  qflex_deinitialize_counts();
}

int64_t QEMU_getCyclesLeft(void)
{
    return cyclesLeft;
}

static int qemu_stopped = 0;

//[???]Not sure what this does if there is a simulation_break, shouldn't there be a simulation_resume?
bool QEMU_quit_simulation(const char * msg)
{
    if (!qemu_stopped) {
        flexus_is_simulating = 0;
        qemu_stopped = 1;

        //[???]it could be pause_all_vcpus(void)
        //Causes the simulation to pause, can be restarted in qemu monitor by calling stop then cont
        //or can be restarted by calling resume_all_vcpus();
        //looking at it some functtions in vl.c might be useful
        printf("Exiting because of break_simulation\n");
        printf("With exit message: %s\n", msg);

        Error* error_msg = NULL;
        error_setg(&error_msg, "%s", msg);
        qmp_quit(&error_msg);
        error_free(error_msg);
    }

    //qemu_system_suspend();//from vl.c:1940//doesn't work at all
    //calls pause_all_vcpus(), and some other stuff.
    //For QEMU to know that they are paused I think.
    //qemu_system_suspend_request();//might be better from vl.c:1948
    //sort of works, but then resets cpus I think


    //I have not found anything that lets you send a message when you pause the simulation, but there can be a wakeup messsage.
    //in vl.c
//    int num_cpus = smp_cpus;
#ifdef CONFIG_DEBUG_LIBQFLEX
    printf ("----------API-OUTPUT----------\n");

    printf ("FETCH ops in io space:         %9i\n",debugStats[FETCH_IO_MEM_OP]);
    printf ("FETCH ops in RAM/ROM:          %9i\n",debugStats[FETCH_MEM_OP]);
    printf ("non fetch op in io space:      %9i\n",debugStats[NON_FETCH_IO_MEM_OP]);
    printf ("Ops not in io space:           %9i\n",debugStats[NON_IO_MEM_OP]);
    printf ("num cpu memory transactions:   %9i\n",debugStats[CPUMEMTRANS]);
    printf ("num ALL CALLBACKS:             %9i\n",debugStats[ALL_CALLBACKS]);
    printf ("num GENERIC CALLBACKS:         %9i\n",debugStats[ALL_GENERIC_CALLBACKS]);
    printf ("num NON_EXISTING_EVENT:        %9i\n",debugStats[NON_EXISTING_EVENT]);


    printf ("----------QEMU-OUTPUT----------\n");
    printf ("Transactions:       %9i\n",debugStats[NUM_DMA_ALL]);

    printf ("CALLBACKS:          %9i\n",debugStats[QEMU_CALLBACK_CNT]);
    printf ("                    %9s\t%9s\t%9s\n","USER","OS","ALL");

    printf ("Total Instructions: %9i\t%9i\t%9i\n",debugStats[USER_INSTR_CNT]
                                                   ,debugStats[OS_INSTR_CNT]
                                                   ,debugStats[ALL_INSTR_CNT]);

    printf ("Total Fetches:      %9i\t%9i\t%9i\n",debugStats[USER_FETCH_CNT]
                                                   ,debugStats[OS_FETCH_CNT]
                                                   ,debugStats[ALL_FETCH_CNT]);

    printf ("Loads:              %9i\t%9i\t%9i\n",debugStats[LD_USER_CNT]
                                                   ,debugStats[LD_OS_CNT]
                                                   ,debugStats[LD_ALL_CNT]);

    printf ("Stores:             %9i\t%9i\t%9i\n",debugStats[ST_USER_CNT]
                                                   ,debugStats[ST_OS_CNT]
                                                   ,debugStats[ST_ALL_CNT]);

    printf ("Cache Ops:          %9i\t%9i\t%9i\n",debugStats[CACHEOPS_USER_CNT]
                                                   ,debugStats[CACHEOPS_OS_CNT]
                                                   ,debugStats[CACHEOPS_ALL_CNT]);

    printf ("Transactions:       %9i\t%9i\t%9i\n",debugStats[NUM_TRANS_USER]
                                                 ,debugStats[NUM_TRANS_OS]
                                                   ,debugStats[NUM_TRANS_ALL]);

#endif

    return true;
}



uint64_t QEMU_get_instruction_count(int cpu_number, int isUser) {

    if (isUser == USER_INSTR ){
        return QEMU_instruction_counts_user[cpu_number];
    } else if (isUser == OS_INSTR ){
        return QEMU_instruction_counts_OS[cpu_number];
    } else {
        return QEMU_instruction_counts[cpu_number];
    }
}

void QEMU_increment_instruction_count(int cpu_number, int isUser) {

    if(isUser == USER_INSTR){
        QEMU_instruction_counts_user[cpu_number]++;
    } else {
        QEMU_instruction_counts_OS[cpu_number]++;
    }
    QEMU_instruction_counts[cpu_number]++;
    QEMU_total_instruction_count++;

}

uint64_t QEMU_get_total_instruction_count(void) {
  return QEMU_total_instruction_count;
}

void QEMU_cpu_set_quantum(const int * val)
{
#ifdef CONFIG_QUANTUM
    /* Msutherl:
     * - I believe this should be removed, b/c the
     *   quantum is always set from QEMU cmd line.
     * - Removed for now.
     * - IF we want to keep it, use the interface in
     *   qemu's cpus.c file to set:
     *      quantum_state.quantum_value = *val
     */
    fprintf(stderr, "----- ERROR in: %s:%d, called QEMU_cpu_set_quantum after functionality removed.-----\n",__FILE__,__LINE__);
    exit(-1);
    /*
    if (*val > 0)
        quantum_value = *val;
    */
#endif
}
#ifdef __cplusplus
}
#endif


