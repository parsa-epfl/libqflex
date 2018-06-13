#ifndef QEMU_API_H
#define QEMU_API_H
#include <inttypes.h>
#include <stdbool.h>

#define OS_INSTR 0
#define USER_INSTR 1
#define BOTH_INSTR 2

void advance_qemu(void);


///
/// Data structures declarations
///

typedef uint64_t cycles_t;
typedef uint64_t physical_address_t;
typedef uint64_t logical_address_t;
typedef void* conf_class_t;
typedef int exception_type_t;

// things for api.cpp:

struct conf_object {
        char *name;
	void *object; // pointer to the struct in question
	enum { // what kind of QEMU struct does it represent
		QEMU_CPUState, // add new types as necessary
		QEMU_AddressSpace,
		QEMU_NetworkDevice
	} type;
};
typedef struct conf_object conf_object_t;
typedef conf_object_t processor_t;

typedef enum {
     QMP_FLEXUS_PRINTCYCLECOUNT = 0,
     QMP_FLEXUS_SETSTOPCYCLE,
     QMP_FLEXUS_SETSTATINTERVAL,
     QMP_FLEXUS_SETREGIONINTERVAL,
     QMP_FLEXUS_SETBREAKCPU,
     QMP_FLEXUS_SETBREAKINSN,
     QMP_FLEXUS_SETPROFILEINTERVAL,
     QMP_FLEXUS_SETTIMESTAMPINTERVAL,
     QMP_FLEXUS_PRINTPROFILE,
     QMP_FLEXUS_RESETPROFILE,
     QMP_FLEXUS_WRITEPROFILE,
     QMP_FLEXUS_PRINTCONFIGURATION,
     QMP_FLEXUS_WRITECONFIGURATION,
     QMP_FLEXUS_PARSECONFIGURATION,
     QMP_FLEXUS_SETCONFIGURATION,
     QMP_FLEXUS_PRINTMEASUREMENT,
     QMP_FLEXUS_LISTMEASUREMENTS,
     QMP_FLEXUS_WRITEMEASUREMENT,
     QMP_FLEXUS_ENTERFASTMODE,
     QMP_FLEXUS_LEAVEFASTMODE,
     QMP_FLEXUS_QUIESCE,
     QMP_FLEXUS_DOLOAD,
     QMP_FLEXUS_DOSAVE,
     QMP_FLEXUS_BACKUPSTATS,
     QMP_FLEXUS_SAVESTATS,
     QMP_FLEXUS_RELOADDEBUGCFG,
     QMP_FLEXUS_ADDDEBUGCFG,
     QMP_FLEXUS_SETDEBUG,
     QMP_FLEXUS_ENABLECATEGORY,
     QMP_FLEXUS_DISABLECATEGORY,
     QMP_FLEXUS_LISTCATEGORIES,
     QMP_FLEXUS_ENABLECOMPONENT,
     QMP_FLEXUS_DISABLECOMPONENT,
     QMP_FLEXUS_LISTCOMPONENTS,
     QMP_FLEXUS_PRINTDEBUGCONFIGURATION,
     QMP_FLEXUS_WRITEDEBUGCONFIGURATION,
     QMP_FLEXUS_TERMINATESIMULATION,
     QMP_FLEXUS_LOG,
     QMP_FLEXUS_PRINTMMU,
} qmp_flexus_cmd_t;

typedef enum {
	QEMU_Class_Kind_Vanilla,
	QEMU_Class_Kind_Pseudo,
	QEMU_Class_Kind_Session
} class_data_t;


typedef enum {
    GENERAL = 0,
    SPECIAL,
    FLOATING_POINT,
    EXCEPTION_LINK,
    STACK_POINTER,
    SAVED_PROGRAM_STATUS,
    PSTATE,
    SYSTEM,
    FPCR,
    FPSR,
} arm_register_t;

typedef enum {
	QEMU_Set_Ok
} set_error_t;

typedef enum {
	QEMU_Trans_Load,
	QEMU_Trans_Store,
	QEMU_Trans_Instr_Fetch,
	QEMU_Trans_Prefetch,
	QEMU_Trans_Cache
} mem_op_type_t;

typedef enum {
	QEMU_PE_No_Exception = 1025,
	QEMU_PE_Code_Break,
	QEMU_PE_Silent_Break,
	QEMU_PE_Inquiry_Outside_Memory,
	QEMU_PE_Inquiry_Unhandled,
	QEMU_PE_IO_Not_Taken,
	QEMU_PE_IO_Error,
	QEMU_PE_Interrupt_Break,
	QEMU_PE_Interrupt_Break_Take_Now,
	QEMU_PE_Exception_Break,
	QEMU_PE_Hap_Exception_Break,
	QEMU_PE_Stall_Cpu,
	QEMU_PE_Locked_Memory,
	QEMU_PE_Return_Break,
	QEMU_PE_Instruction_Finished,
	QEMU_PE_Default_Semantics,
	QEMU_PE_Ignore_Semantics,
	QEMU_PE_Speculation_Failed,
	QEMU_PE_Invalid_Address,
	QEMU_PE_MAI_Return,
	QEMU_PE_Last
} pseudo_exceptions_t;

typedef enum {
	QEMU_Initiator_Illegal         = 0x0,    /* catch uninitialized */
	QEMU_Initiator_CPU             = 0x1000,
	QEMU_Initiator_CPU_V9          = 0x1100,
	QEMU_Initiator_CPU_UII         = 0x1101,
	QEMU_Initiator_CPU_UIII        = 0x1102,
	QEMU_Initiator_CPU_UIV         = 0x1103,
	QEMU_Initiator_CPU_UT1         = 0x1104, /* 1105, 1106 internal */
	QEMU_Initiator_CPU_X86         = 0x1200,
	QEMU_Initiator_CPU_PPC         = 0x1300,
	QEMU_Initiator_CPU_Alpha       = 0x1400,
	QEMU_Initiator_CPU_IA64        = 0x1500,
	QEMU_Initiator_CPU_MIPS        = 0x1600,
	QEMU_Initiator_CPU_MIPS_RM7000 = 0x1601,
	QEMU_Initiator_CPU_MIPS_E9000  = 0x1602,
	QEMU_Initiator_CPU_ARM         = 0x1700,
	QEMU_Initiator_Device          = 0x2000,
	QEMU_Initiator_PCI_Device      = 0x2010,
	QEMU_Initiator_Cache           = 0x3000, /* The transaction is a cache
											   transaction as defined by
											   g-cache */
	QEMU_Initiator_Other           = 0x4000  /* initiator == NULL */
} ini_type_t;

typedef enum {
  QEMU_Non_Branch = 0,
  QEMU_Conditional_Branch = 1,
  QEMU_Unconditional_Branch = 2,
  QEMU_Call_Branch = 3,
  QEMU_Return_Branch = 4,
  QEMU_Last_Branch_Type = 5,
  QEMU_BRANCH_TYPE_COUNT
} branch_type_t;

typedef enum {
    FETCH_IO_MEM_OP = 0,
    FETCH_MEM_OP,
    NON_FETCH_IO_MEM_OP,
    NON_IO_MEM_OP,
    CPUMEMTRANS,
    ALL_CALLBACKS,
    ALL_GENERIC_CALLBACKS,
    NON_EXISTING_EVENT,

    USER_INSTR_CNT,
    OS_INSTR_CNT,
    ALL_INSTR_CNT,

    USER_FETCH_CNT,
    OS_FETCH_CNT,
    ALL_FETCH_CNT,

    LD_USER_CNT,
    LD_OS_CNT,
    LD_ALL_CNT,

    ST_USER_CNT,
    ST_OS_CNT,
    ST_ALL_CNT,

    CACHEOPS_USER_CNT,
    CACHEOPS_OS_CNT,
    CACHEOPS_ALL_CNT,

    NUM_TRANS_USER,
    NUM_TRANS_OS,
    NUM_TRANS_ALL,

    NUM_DMA_ALL,

    QEMU_CALLBACK_CNT,

    ALL_DEBUG_TYPE, // should be last

} debug_types;
//Original definition in /home/parsacom/tools/simics/src/include/simics/core/memory.h
struct generic_transaction {
        void *cpu_state;// (CPUState*) state of the CPU source of the transaction
	conf_object_t *ini_ptr; // note: for efficiency, arrange struct from
	char *real_address;     // largest datatype to smallest to avoid
	uint64_t bytes;         // unnecessary padding.
        logical_address_t pc; // QEMU pc not updated regularly, need to send pc
	logical_address_t logical_address;
	physical_address_t physical_address;
    unsigned  size;
	mem_op_type_t type;
	ini_type_t ini_type;
	exception_type_t exception;
        branch_type_t branch_type;
        unsigned int annul : 1;// annul the delay slot or not
	unsigned int atomic:1;
	unsigned int inquiry:1;
	unsigned int may_stall:1;
	unsigned int speculative:1;
	unsigned int ignore:1;
	unsigned int inverse_endian:1;
};
typedef struct generic_transaction generic_transaction_t;

typedef enum {
	QEMU_Val_Invalid  = 0,
	QEMU_Val_String   = 1,
	QEMU_Val_Integer  = 2,
	QEMU_Val_Floating = 3,
	QEMU_Val_Nil      = 6,
	QEMU_Val_Object   = 7,
	QEMU_Val_Boolean  = 9
} attr_kind_t;

struct attr_value {
	attr_kind_t kind;
	union {
		const char *string;
		int64_t integer;
		int64_t boolean;
		double floating;
		conf_object_t *object;
	} u;
};
typedef struct attr_value attr_value_t;

// part of MAI
typedef enum instruction_error {
	QEMU_IE_OK = 0,
	QEMU_IE_Unresolved_Dependencies,
	QEMU_IE_Speculative,
	QEMU_IE_Stalling,
	QEMU_IE_Not_Inserted,      /* trying to execute or squash an 
							 instruction that is inserted. */ 
	QEMU_IE_Exception,         /* (SPARC-V9 only) */
	QEMU_IE_Fault = QEMU_IE_Exception, 
	QEMU_IE_Trap,              /* (X86 only) Returned if a trap is 
							 encountered */
	QEMU_IE_Interrupt,         /* (X86 only) Returned if an interrupt is 
							 waiting and interrupts are enabled */

	QEMU_IE_Sync_Instruction,  /* Returned if sync instruction is 
							 not allowd to execute */
	QEMU_IE_No_Exception,      /* Returned by QEMU_instruction_
							 handle_exception */
	QEMU_IE_Illegal_Interrupt_Point,
	QEMU_IE_Illegal_Exception_Point,
	QEMU_IE_Illegal_Address,
	QEMU_IE_Illegal_Phase,
	QEMU_IE_Interrupts_Disabled,
	QEMU_IE_Illegal_Id,
	QEMU_IE_Instruction_Tree_Full,
	QEMU_IE_Null_Pointer,
	QEMU_IE_Illegal_Reg,
	QEMU_IE_Invalid,
	QEMU_IE_Out_of_Order_Commit, 
	QEMU_IE_Retired_Instruction, /* try to squash a retiring instruction */
	QEMU_IE_Not_Committed,       /* Returned by QEMU_instruction_end */
	QEMU_IE_Code_Breakpoint,
	QEMU_IE_Mem_Breakpoint,
	QEMU_IE_Step_Breakpoint,
	QEMU_IE_Hap_Breakpoint
} instruction_error_t;

typedef enum {
	QEMU_CPU_Mode_User,
	QEMU_CPU_Mode_Supervisor,
	QEMU_CPU_Mode_Hypervisor
} processor_mode_t;

typedef enum {
  QEMU_Instruction_Cache = 1,
  QEMU_Data_Cache = 2,
  QEMU_Prefetch_Buffer = 4
} cache_type_t;

typedef enum {
  QEMU_Invalidate_Cache = 0,
  QEMU_Clean_Cache = 1,
  QEMU_Flush_Cache = 2,
  QEMU_Prefetch_Cache = 3
} cache_maintenance_op_t;

typedef struct set_and_way_data{
  uint32_t set;
  uint32_t way;
} set_and_way_data_t;

/* little structure to hold an address range */
/* it is inclusive: if start == end, the range contains start */
typedef struct address_range{
  physical_address_t start_paddr;
  physical_address_t end_paddr;
} address_range_t;

typedef struct memory_transaction_sparc_specific {
	unsigned int cache_virtual:1;
	unsigned int cache_physical:1;
	unsigned int priv:1;
	uint8_t	     address_space;
	uint8_t        prefetch_fcn;
} memory_transaction_sparc_specific_t;

typedef struct memory_transaction_i386_specific {
  processor_mode_t mode;
} memory_transaction_i386_specific_t;

typedef struct memory_transaction_arm_specific {
  // wether or not the transaction comes from the user
  unsigned int user:1;
} memory_transaction_arm_specific_t;

typedef struct memory_transaction {
  generic_transaction_t s;
  unsigned int io:1;
  cache_type_t cache;// cache to operate on
  cache_maintenance_op_t cache_op;// operation to perform on cache
  int line:1; // 1 for line, 0 for whole cache
  int data_is_set_and_way : 1;// wether or not the operation provides set&way or address (range)
  union{
    set_and_way_data_t set_and_way;
    address_range_t addr_range;// same start and end addresses for not range operations
  };

  union{
    memory_transaction_sparc_specific_t sparc_specific;
    memory_transaction_i386_specific_t i386_specific;
    memory_transaction_arm_specific_t arm_specific;
  };
} memory_transaction_t;

typedef enum {
	QEMU_DI_Instruction,
	QEMU_DI_Data
} data_or_instr_t;

#ifdef FLEXUS_TARGET_ARM
typedef struct armInterface {
    //This is the interface for a Sparc CPU in QEMU. The interface should provide the following functions:
    //uint64_t read_fp_register_x(conf_object_t *cpu, int reg)
    //void write_fp_register_x(conf_object_t *cpu, int reg, uint64 value);
    //uint64_t read_global_register(conf_object_t *cpu, int globals, int reg);
    //uint64_t read_window_register(conf_object_t *cpu, int window, int reg);
    //exception_type_t access_asi_handler(conf_object_t *cpu, v9_memory_transaction_t *mem_op);
} armInterface_t;
typedef struct {
    //This is the interface for a Sparc mmu in QEMU. The interface should provide the following functions:
        //exception_type_t (*logical_to_physical)(conf_object_t *mmu_obj, v9_memory_transaction_t *);
} mmu_interface_t;

typedef enum {
        ARM_Access_Normal,
        ARM_Access_Normal_FP,
        ARM_Access_Double_FP, /* ldd/std */
        ARM_Access_Short_FP,
        ARM_Access_FSR,
        ARM_Access_Atomic,
        ARM_Access_Atomic_Load,
        ARM_Access_Prefetch,
        ARM_Access_Partial_Store,
        ARM_Access_Ldd_Std_1,
        ARM_Access_Ldd_Std_2,
        ARM_Access_Block,
        ARM_Access_Internal1
} arm_access_type_t;

typedef struct arm_memory_transaction {
        generic_transaction_t s;
        unsigned              cache_virtual:1;
        unsigned              cache_physical:1;
        unsigned              side_effect:1;
        unsigned              priv:1;
        unsigned              red:1;
        unsigned              hpriv:1;
        unsigned              henb:1;
        /* Because of a bug in the Sun Studio12 C compiler, bit fields must not
 *            be followed by members with alignment smaller than 32 bit.
 *                       See bug 9151. */
        uint32_t address_space;
        uint8_t                 prefetch_fcn;
        arm_access_type_t   access_type;

        /* if non-zero, the id needed to calculate the program counter */
        intptr_t turbo_miss_id;
} arm_memory_transaction_t;
#else
//TODO: Interfaces for Sparc cpu and mmu
typedef struct sparc_v9_interface {
	//This is the interface for a Sparc CPU in QEMU. The interface should provide the following functions:
	//uint64_t read_fp_register_x(conf_object_t *cpu, int reg)
	//void write_fp_register_x(conf_object_t *cpu, int reg, uint64 value);
	//uint64_t read_global_register(conf_object_t *cpu, int globals, int reg);
	//uint64_t read_window_register(conf_object_t *cpu, int window, int reg);
	//exception_type_t access_asi_handler(conf_object_t *cpu, v9_memory_transaction_t *mem_op);
} sparc_v9_interface_t;

typedef struct {
	//This is the interface for a Sparc mmu in QEMU. The interface should provide the following functions:
        //exception_type_t (*logical_to_physical)(conf_object_t *mmu_obj, v9_memory_transaction_t *);
} mmu_interface_t;

typedef enum {
        V9_Access_Normal,
        V9_Access_Normal_FP,
        V9_Access_Double_FP, /* ldd/std */
        V9_Access_Short_FP,
        V9_Access_FSR,
        V9_Access_Atomic,
        V9_Access_Atomic_Load,
        V9_Access_Prefetch,
        V9_Access_Partial_Store,
        V9_Access_Ldd_Std_1,
        V9_Access_Ldd_Std_2,
        V9_Access_Block,
        V9_Access_Internal1
} sparc_access_type_t;

typedef struct v9_memory_transaction {
        generic_transaction_t s;
        unsigned              cache_virtual:1;
        unsigned              cache_physical:1;
        unsigned              side_effect:1;
        unsigned              priv:1;
        unsigned              red:1;
        unsigned              hpriv:1;
        unsigned              henb:1;
        /* Because of a bug in the Sun Studio12 C compiler, bit fields must not
 *            be followed by members with alignment smaller than 32 bit.
 *                       See bug 9151. */
        uint32_t address_space;
        uint8_t                 prefetch_fcn;
        sparc_access_type_t   access_type;

        /* if non-zero, the id needed to calculate the program counter */
        intptr_t turbo_miss_id;
} v9_memory_transaction_t;

#endif //FLEXUS_TARGET_IS(v9)

///
/// State interaction API
///
/// Get/set machine state from the Flexus.
///

typedef void (*CPU_READ_REGISTER_PROC)(void* env_ptr, int reg_index, unsigned *reg_size, void *data_out);
typedef void (*CPU_WRITE_REGISTER_PROC)(void* env_ptr, int reg_index, unsigned *reg_size, uint64_t value);
typedef uint64_t (*READREG_PROC)(void *cs_, int reg_idx, int reg_type);
typedef physical_address_t (*MMU_LOGICAL_TO_PHYSICAL_PROC)(void *cs, logical_address_t va);
typedef uint64_t (*CPU_GET_PROGRAM_COUNTER_PROC)(void* cs);
typedef uint64_t (*CPU_GET_INSTRUCTION_PROC)(void* cs, uint64_t* addr);
typedef void* (*CPU_GET_ADDRESS_SPACE_PROC)(void* cs);
typedef int (*CPU_PROC_NUM_PROC)(void* cs);
typedef void (*CPU_POP_INDEXES_PROC)(int* indexes);
typedef conf_object_t* (*QEMU_GET_PHYS_MEMORY_PROC)(conf_object_t* cpu);
typedef conf_object_t* (*QEMU_GET_ETHERNET_PROC)(void);
typedef int (*QEMU_CLEAR_EXCEPTION_PROC)(void);
typedef void (*QEMU_READ_REGISTER_PROC)(conf_object_t *cpu, int reg_index, unsigned* reg_size, void* data_out);
typedef void (*QEMU_WRITE_REGISTER_PROC)(conf_object_t *cpu, int reg_index, unsigned* reg_size, uint64_t value);
typedef uint64_t (*QEMU_READ_REGISTER_BY_TYPE_PROC)(conf_object_t *cpu, int reg_index, int reg_type);
typedef uint64_t (*QEMU_READ_PHYS_MEMORY_PROC)(conf_object_t *cpu, physical_address_t pa, int bytes);
typedef conf_object_t *(*QEMU_GET_PHYS_MEM_PROC)(conf_object_t *cpu);
typedef conf_object_t* (*QEMU_GET_CPU_BY_INDEX_PROC)(int index);
typedef int (*QEMU_GET_PROCESSOR_NUMBER_PROC)(conf_object_t *cpu);
typedef uint64_t (*QEMU_STEP_COUNT_PROC)(conf_object_t *cpu);
typedef int (*QEMU_GET_NUM_CPUS_PROC)(void);
typedef int (*QEMU_GET_NUM_SOCKETS_PROC)(void);
typedef int (*QEMU_GET_NUM_CORES_PROC)(void);
typedef int (*QEMU_GET_NUM_THREADS_PER_CORE_PROC)(void);
typedef int (*QEMU_CPU_GET_SOCKET_ID_PROC)(conf_object_t *cpu);
typedef int (*QEMU_CPU_GET_CORE_ID_PROC)(conf_object_t *cpu);
typedef int (*QEMU_CPU_GET_THREAD_ID_PROC)(conf_object_t *cpu);
typedef conf_object_t* (*QEMU_GET_ALL_PROCESSORS_PROC)(int *numCPUS);
typedef void (*QEMU_CPU_SET_QUANTUM)(const int *val);
typedef int (*QEMU_SET_TICK_FREQUENCY_PROC)(conf_object_t *cpu, double tick_freq);
typedef double (*QEMU_GET_TICK_FREQUENCY_PROC)(conf_object_t *cpu);
typedef uint64_t (*QEMU_GET_PROGRAM_COUNTER_PROC)(conf_object_t *cpu);
typedef uint64_t (*QEMU_GET_INSTRUCTION_PROC)(conf_object_t *cpu, uint64_t *addr);
typedef void (*QEMU_INCREMENT_DEBUG_STAT_PROC)(int val);

typedef physical_address_t (*QEMU_LOGICAL_TO_PHYSICAL_PROC)(conf_object_t *cpu,
					data_or_instr_t fetch, logical_address_t va);
typedef bool (*QEMU_BREAK_SIMULATION_PROC)(const char* msg);

typedef int (*QEMU_IS_STOPPED_PROC)(void);

typedef void (*QEMU_SET_SIMULATION_TIME_PROC)(int* time);
typedef void (*QEMU_GET_SIMULATION_TIME_PROC)(int* time);

typedef void (*QEMU_FLUSH_ALL_CACHES_PROC)(void);
typedef int (*QEMU_MEM_OP_IS_DATA_PROC)(generic_transaction_t *mop);
typedef int (*QEMU_MEM_OP_IS_WRITE_PROC)(generic_transaction_t *mop);
typedef int (*QEMU_MEM_OP_IS_READ_PROC)(generic_transaction_t *mop);
typedef void (*QEMU_WRITE_PHYS_MEMORY_PROC)(conf_object_t *cpu, physical_address_t pa, unsigned long long value, int bytes);

////For Timing (mai) 
typedef instruction_error_t (*QEMU_INSTRUCTION_HANDLE_INTERRUPT_PROC)(conf_object_t *cpu, pseudo_exceptions_t pendingInterrupt);
typedef int (*QEMU_GET_PENDING_EXCEPTION_PROC)(void);
typedef int (*QEMU_ADVANCE_PROC)(void);
typedef conf_object_t* (*QEMU_GET_OBJECT_PROC)(const char *name);
////ALEX - end 

//For Timing
typedef int (*QEMU_CPU_EXEC_PROC)(conf_object_t *cpu);

/// Higher order API functions
typedef int (*QEMU_IS_IN_SIMULATION_PROC)(void);
typedef void (*QEMU_TOGGLE_SIMULATION_PROC)(int enable);
typedef void (*QEMU_FLUSH_TB_CACHE_PROC)(void);

// Get the instruction count for the given processor
typedef uint64_t (*QEMU_GET_INSTRUCTION_COUNT_PROC)(int cpu_number, int isUser);
typedef uint64_t (*QEMU_GET_INSTRUCTION_COUNT_PROC2)(int cpu_number, int isUser);

#ifndef CONFIG_FLEXUS
extern CPU_READ_REGISTER_PROC cpu_read_register;
extern CPU_WRITE_REGISTER_PROC cpu_write_register;
extern READREG_PROC readReg;
extern MMU_LOGICAL_TO_PHYSICAL_PROC mmu_logical_to_physical;
extern CPU_GET_INSTRUCTION_PROC cpu_get_instruction;
extern CPU_GET_PROGRAM_COUNTER_PROC cpu_get_program_counter;
extern CPU_GET_ADDRESS_SPACE_PROC cpu_get_address_space_flexus;
extern CPU_PROC_NUM_PROC cpu_proc_num;
extern CPU_POP_INDEXES_PROC cpu_pop_indexes;
extern QEMU_GET_PHYS_MEMORY_PROC QEMU_get_phys_memory;
extern QEMU_GET_ETHERNET_PROC QEMU_get_ethernet;
extern QEMU_CLEAR_EXCEPTION_PROC QEMU_clear_exception;
extern QEMU_READ_REGISTER_PROC QEMU_read_register;
extern QEMU_WRITE_REGISTER_PROC QEMU_write_register;
extern QEMU_READ_REGISTER_BY_TYPE_PROC QEMU_read_register_by_type;
extern QEMU_READ_PHYS_MEMORY_PROC QEMU_read_phys_memory;
extern QEMU_GET_PHYS_MEM_PROC QEMU_get_phys_mem;
extern QEMU_GET_CPU_BY_INDEX_PROC QEMU_get_cpu_by_index;
extern QEMU_GET_PROCESSOR_NUMBER_PROC QEMU_get_processor_number;
extern QEMU_STEP_COUNT_PROC QEMU_step_count;
extern QEMU_GET_NUM_CPUS_PROC QEMU_get_num_cpus;
extern QEMU_WRITE_PHYS_MEMORY_PROC QEMU_write_phys_memory;
// return the number of sockets on he motherboard
extern QEMU_GET_NUM_SOCKETS_PROC QEMU_get_num_sockets;
// returns the number of cores per CPU socket
extern QEMU_GET_NUM_CORES_PROC QEMU_get_num_cores;
// return the number of native threads per core
extern QEMU_GET_NUM_THREADS_PER_CORE_PROC QEMU_get_num_threads_per_core;
// return the id of the socket of the processor
extern QEMU_CPU_GET_SOCKET_ID_PROC QEMU_cpu_get_socket_id;
// return the core id of the processor
extern QEMU_CPU_GET_CORE_ID_PROC QEMU_cpu_get_core_id;
// return the hread id of the processor
extern QEMU_CPU_GET_THREAD_ID_PROC QEMU_cpu_get_thread_id;
// return an array of all processors
// (numSockets * numCores * numthreads CPUs)
extern QEMU_GET_ALL_PROCESSORS_PROC QEMU_get_all_processors;
extern QEMU_CPU_SET_QUANTUM QEMU_cpu_set_quantum;
// set the frequency of a given cpu.
extern QEMU_SET_TICK_FREQUENCY_PROC QEMU_set_tick_frequency;
// get freq of given cpu
extern QEMU_GET_TICK_FREQUENCY_PROC QEMU_get_tick_frequency;
// get the program counter of a given cpu.
extern QEMU_GET_INSTRUCTION_PROC QEMU_get_instruction;
// get the program counter of a given cpu.
extern QEMU_GET_PROGRAM_COUNTER_PROC QEMU_get_program_counter;
extern QEMU_INCREMENT_DEBUG_STAT_PROC QEMU_increment_debug_stat;
// convert a logical address to a physical address.
extern QEMU_LOGICAL_TO_PHYSICAL_PROC QEMU_logical_to_physical;
extern QEMU_BREAK_SIMULATION_PROC QEMU_break_simulation;
extern QEMU_IS_STOPPED_PROC QEMU_is_stopped;
extern QEMU_SET_SIMULATION_TIME_PROC QEMU_setSimulationTime;
extern QEMU_GET_SIMULATION_TIME_PROC QEMU_getSimulationTime;
// dummy function at the moment. should flush the translation cache.
extern QEMU_FLUSH_ALL_CACHES_PROC QEMU_flush_all_caches;
// determine the memory operation type by the transaction struct.
//[???]I assume return true if it is data, false otherwise
extern QEMU_MEM_OP_IS_DATA_PROC QEMU_mem_op_is_data;
//[???]I assume return true if it is write, false otherwise
extern QEMU_MEM_OP_IS_WRITE_PROC QEMU_mem_op_is_write;
//[???]I assume return true if it is read, false otherwise
extern QEMU_MEM_OP_IS_READ_PROC QEMU_mem_op_is_read;
extern QEMU_INSTRUCTION_HANDLE_INTERRUPT_PROC QEMU_instruction_handle_interrupt;
extern QEMU_GET_PENDING_EXCEPTION_PROC QEMU_get_pending_exception;
extern QEMU_ADVANCE_PROC QEMU_advance;
extern QEMU_GET_OBJECT_PROC QEMU_get_object;
extern QEMU_CPU_EXEC_PROC QEMU_cpu_exec_proc;
extern QEMU_IS_IN_SIMULATION_PROC QEMU_is_in_simulation;
extern QEMU_TOGGLE_SIMULATION_PROC QEMU_toggle_simulation;
extern QEMU_FLUSH_TB_CACHE_PROC QEMU_flush_tb_cache;
extern QEMU_GET_INSTRUCTION_COUNT_PROC QEMU_get_instruction_count;
#else
// query the content/size of a register
// if reg_size != NULL, write the size of the register (in bytes) in reg_size
// if data_out != NULL, write the content of the register in data_out
void cpu_read_register( void *env_ptr, int reg_index, unsigned *reg_size, void *data_out );
void cpu_write_register( void *env_ptr, int reg_index, unsigned *reg_size, uint64_t value );
uint64_t readReg(void *cs_, int reg_idx, int reg_type); 
physical_address_t mmu_logical_to_physical(void *cs, logical_address_t va);
uint32_t cpu_get_instruction(void *cs, uint64_t* addr);
uint64_t cpu_get_program_counter(void *cs);
void* cpu_get_address_space_flexus(void *cs);              // Changed the name here
int cpu_proc_num(void *cs);
void cpu_pop_indexes(int *indexes);
conf_object_t *QEMU_get_phys_memory(conf_object_t *cpu);
conf_object_t *QEMU_get_ethernet(void);
//[???]based on name it clears exceptions
int QEMU_clear_exception(void);


// read an arbitrary register.
// query the content/size of a register
// if reg_size != NULL, write the size of the register (in bytes) in reg_size
// if data_out != NULL, write the content of the register in data_out
void QEMU_read_register(conf_object_t *cpu,
			int reg_index,
			unsigned *reg_size,
			void *data_out);

void QEMU_write_register(conf_object_t *cpu,
			int reg_index,
			unsigned *reg_size,
			uint64_t value);
uint64_t QEMU_read_register_by_type(conf_object_t *cpu, int reg_index, int reg_type);

// read an arbitrary physical memory address.
uint64_t QEMU_read_phys_memory(conf_object_t *cpu, 
								physical_address_t pa, int bytes);
void QEMU_write_phys_memory(conf_object_t *cpu, physical_address_t pa, unsigned long long value, int bytes);
// get the physical memory for a given cpu.
conf_object_t *QEMU_get_phys_mem(conf_object_t *cpu);
// return a conf_object_t of the cpu in question.
conf_object_t *QEMU_get_cpu_by_index(int index);

// return an int specifying the processor number of the cpu.
int QEMU_get_processor_number(conf_object_t *cpu);

// how many instructions have been executed since the start of QEMU for a CPU
uint64_t QEMU_step_count(conf_object_t *cpu);

// return an array of all processorsthe totaltthe total number of processors
// (numSockets * numCores * numthreads CPUs)
int QEMU_get_num_cpus(void);

// return the number of sockets on he motherboard
int QEMU_get_num_sockets(void);

// returns the number of cores per CPU socket
int QEMU_get_num_cores(void);

// return the number of native threads per core
int QEMU_get_num_threads_per_core(void);

// return the id of the socket of the processor
int QEMU_cpu_get_socket_id(conf_object_t *cpu);

// return the core id of the processor
int QEMU_cpu_get_core_id(conf_object_t *cpu);

// return the hread id of the processor
int QEMU_cpu_get_thread_id(conf_object_t *cpu);

// return an array of all processors
// (numSockets * numCores * numthreads CPUs)
conf_object_t *QEMU_get_all_processors(int *numCPUs);
void QEMU_cpu_set_quantum(const int * val);
// set the frequency of a given cpu.
int QEMU_set_tick_frequency(conf_object_t *cpu, double tick_freq);
// get freq of given cpu
double QEMU_get_tick_frequency(conf_object_t *cpu);

uint32_t QEMU_get_instruction(conf_object_t *cpu,uint64_t* addr);

// get the program counter of a given cpu.
uint64_t QEMU_get_program_counter(conf_object_t *cpu);
void QEMU_increment_debug_stat(int val);

// convert a logical address to a physical address.
physical_address_t QEMU_logical_to_physical(conf_object_t *cpu, 
					data_or_instr_t fetch, logical_address_t va);
bool QEMU_break_simulation(const char *msg);

int QEMU_is_stopped(void);

void QEMU_getSimulationTime(int *time);
void QEMU_setSimulationTime(int *time);

// dummy function at the moment. should flush the translation cache.
void QEMU_flush_all_caches(void);
// determine the memory operation type by the transaction struct.
//[???]I assume return true if it is data, false otherwise
int QEMU_mem_op_is_data(generic_transaction_t *mop);
//[???]I assume return true if it is write, false otherwise
int QEMU_mem_op_is_write(generic_transaction_t *mop);
//[???]I assume return true if it is read, false otherwise
int QEMU_mem_op_is_read(generic_transaction_t *mop);

//ALEX - begin 
////For Timing (mai) 
instruction_error_t QEMU_instruction_handle_interrupt(conf_object_t *cpu, pseudo_exceptions_t pendingInterrupt); 
int QEMU_get_pending_exception(void); 
int QEMU_advance(void); 
conf_object_t *QEMU_get_object(const char *name);	//generic function to get a pointer to a QEMU object by name
////ALEX - end 
//

//NOOSHIN: begin
// cpu must point to a CPUState object
int get_info(void *cpu);

int QEMU_cpu_exec_proc(conf_object_t *cpu);
//NOOSHIN: end

int QEMU_is_in_simulation(void);

void QEMU_toggle_simulation(int enable);

// Return the number of instructions to simulate for.
// If it is less than 0, it means that it shouldn't stop.
int64_t QEMU_get_simulation_length(void);

void QEMU_flush_tb_cache(void);

// Get the instruction count for the given processor
uint64_t QEMU_get_instruction_count(int cpu_number, int isUser);

// Get the total instruction count for all the processors.
uint64_t QEMU_get_total_instruction_count(void);
#endif
///
/// Callback API
///
/// The callback API is used to send event notifications
/// to the Flexus when using trace mode.
///

// callback function types.
// 
// naming convention:
// ------------------
// i - int
// I - int64_t
// e - exception_type_t
// o - class data (void*)
// s - string
// m - generic_transaction_t*
// c - conf_object_t*
// v - void*
typedef void (*cb_func_void)(void);
typedef void (*cb_func_noc_t)(void *, conf_object_t *);
typedef void (*cb_func_noc_t2)(void*, void *, conf_object_t *);
typedef void (*cb_func_nocI_t)(void *, conf_object_t *, int64_t);
typedef void (*cb_func_nocI_t2)(void*, void *, conf_object_t *, int64_t);
typedef void (*cb_func_nocIs_t)(void *, conf_object_t *, int64_t, char *);
typedef void (*cb_func_nocIs_t2)(void *, void *, conf_object_t *, int64_t, char *);
typedef void (*cb_func_noiiI_t)(void *, int, int, int64_t);
typedef void (*cb_func_noiiI_t2)(void *, void *, int, int, int64_t);

typedef void (*cb_func_ncm_t)(
		  conf_object_t *
		, memory_transaction_t *
		);
typedef void (*cb_func_ncm_t2)(
          void *
		, conf_object_t *
		, memory_transaction_t *
		);
typedef void (*cb_func_nocs_t)(void *, conf_object_t *, char *);
typedef void (*cb_func_nocs_t2)(void *, void *, conf_object_t *, char *);


typedef struct {
	void *class_data;
	conf_object_t *obj;
} QEMU_noc;

typedef struct {
	void *class_data;
	conf_object_t *obj;
	int64_t bigint;
} QEMU_nocI;

typedef struct {
	void *class_data;
	conf_object_t *obj;
	char *string;
	int64_t bigint;
} QEMU_nocIs;

typedef struct {
	void *class_data;
	int integer0;
	int integer1;
	int64_t bigint;
} QEMU_noiiI;

typedef struct {
	conf_object_t *space;
	memory_transaction_t *trans;
} QEMU_ncm;

typedef struct {
	void *class_data;
	conf_object_t *obj;
	char *string;
} QEMU_nocs;
typedef union {
	QEMU_noc	*noc;
	QEMU_nocIs	*nocIs;
	QEMU_nocI   *nocI;
	QEMU_noiiI	*noiiI;
	QEMU_nocs	*nocs;
	QEMU_ncm	*ncm;
} QEMU_callback_args_t;


typedef enum {
	QEMU_config_ready,
    QEMU_continuation,
    QEMU_simulation_stopped,
    QEMU_asynchronous_trap,
    QEMU_exception_return,
    QEMU_magic_instruction,
    QEMU_ethernet_frame,
    QEMU_ethernet_network_frame,
    QEMU_periodic_event,
    QEMU_xterm_break_string,
    QEMU_gfx_break_string,
    QEMU_cpu_mem_trans,
	QEMU_dma_mem_trans,
    QEMU_callback_event_count // MUST BE LAST.
} QEMU_callback_event_t;

struct QEMU_callback_container {
	uint64_t id;
	void *obj;
	void *callback;
	struct QEMU_callback_container *next;
};
typedef struct QEMU_callback_container QEMU_callback_container_t;

struct QEMU_callback_table {
	uint64_t next_callback_id;
	QEMU_callback_container_t *callbacks[QEMU_callback_event_count];
};
typedef struct QEMU_callback_table QEMU_callback_table_t;

#define QEMUFLEX_GENERIC_CALLBACK -1

typedef int (*QEMU_INSERT_CALLBACK_PROC)( int cpu_id, QEMU_callback_event_t event, void* obj, void* fun);
typedef void (*QEMU_DELETE_CALLBACK_PROC)( int cpu_id, QEMU_callback_event_t event, uint64_t callback_id);

#ifndef CONFIG_FLEXUS
// insert a callback specific for the given cpu or -1 for a generic callback
extern QEMU_INSERT_CALLBACK_PROC QEMU_insert_callback;

// delete a callback specific for the given cpu or -1 for a generic callback
extern QEMU_DELETE_CALLBACK_PROC QEMU_delete_callback;
#else
// insert a callback specific for the given cpu or -1 for a generic callback
int QEMU_insert_callback( int cpu_id, QEMU_callback_event_t event, void* obj, void* fun);
// delete a callback specific for the given cpu or -1 for a generic callback
void QEMU_delete_callback( int cpu_id, QEMU_callback_event_t event, uint64_t callback_id);
#endif // CONFIG_FLEXUS

///
/// QEMU QEMUFLEX internals
///

// Initialize the callback tables for every processor and also the
// different counts.
void QEMU_initialize(bool timing_mode);
// Free the callback table
void QEMU_shutdown(void);

// Initialize the callback tables for every processor
// Must be called at QEMU startup, before initializing Flexus
void QEMU_setup_callback_tables(void);
// Free the allocated memory for the callback tables
void QEMU_free_callback_tables(void);
// execute a callback trigered by the given cpu id or -1 for a generic callback
void QEMU_execute_callbacks(
		  int cpu_id,
		  QEMU_callback_event_t event,
		  QEMU_callback_args_t *event_data
		);

// Initialize to 0 the instruction counts for every processor
void QEMU_initialize_counts(void);
// Free the memory for the counters
void QEMU_deinitialize_counts(void);
// Increment the instruction count for the given cpu
void QEMU_increment_instruction_count(int cpu_number, int isUser);

///
/// Simulator interface function passing
///
typedef struct QFLEX_API_Interface_Hooks {
CPU_READ_REGISTER_PROC cpu_read_register;

CPU_WRITE_REGISTER_PROC cpu_write_register;
READREG_PROC readReg;
MMU_LOGICAL_TO_PHYSICAL_PROC mmu_logical_to_physical;
CPU_GET_PROGRAM_COUNTER_PROC cpu_get_program_counter;
CPU_GET_INSTRUCTION_PROC cpu_get_instruction;
CPU_GET_ADDRESS_SPACE_PROC cpu_get_address_space;
CPU_PROC_NUM_PROC cpu_proc_num;
CPU_POP_INDEXES_PROC cpu_pop_indexes;
QEMU_GET_PHYS_MEMORY_PROC QEMU_get_phys_memory;
QEMU_GET_ETHERNET_PROC QEMU_get_ethernet;
QEMU_CLEAR_EXCEPTION_PROC QEMU_clear_exception;
QEMU_READ_REGISTER_PROC QEMU_read_register;
QEMU_WRITE_REGISTER_PROC QEMU_write_register;
QEMU_READ_REGISTER_BY_TYPE_PROC QEMU_read_register_by_type;
QEMU_READ_PHYS_MEMORY_PROC QEMU_read_phys_memory;
QEMU_GET_PHYS_MEM_PROC QEMU_get_phys_mem;
QEMU_GET_CPU_BY_INDEX_PROC QEMU_get_cpu_by_index;
QEMU_GET_PROCESSOR_NUMBER_PROC QEMU_get_processor_number;
QEMU_STEP_COUNT_PROC QEMU_step_count;
QEMU_GET_NUM_CPUS_PROC QEMU_get_num_cpus;
QEMU_WRITE_PHYS_MEMORY_PROC QEMU_write_phys_memory;
// return the number of sockets on he motherboard
QEMU_GET_NUM_SOCKETS_PROC QEMU_get_num_sockets;

// returns the number of cores per CPU socket
QEMU_GET_NUM_CORES_PROC QEMU_get_num_cores;

// return the number of native threads per core
QEMU_GET_NUM_THREADS_PER_CORE_PROC QEMU_get_num_threads_per_core;

// return the id of the socket of the processor
QEMU_CPU_GET_SOCKET_ID_PROC QEMU_cpu_get_socket_id;

// return the core id of the processor
QEMU_CPU_GET_CORE_ID_PROC QEMU_cpu_get_core_id;

// return the hread id of the processor
QEMU_CPU_GET_THREAD_ID_PROC QEMU_cpu_get_thread_id;

// return an array of all processors
// (numSockets * numCores * numthreads CPUs)
QEMU_GET_ALL_PROCESSORS_PROC QEMU_get_all_processors;

QEMU_CPU_SET_QUANTUM QEMU_cpu_set_quantum;
// set the frequency of a given cpu.
QEMU_SET_TICK_FREQUENCY_PROC QEMU_set_tick_frequency;

// get freq of given cpu
QEMU_GET_TICK_FREQUENCY_PROC QEMU_get_tick_frequency;

// get the converted instruction - endianness
QEMU_GET_INSTRUCTION_PROC QEMU_get_instruction;

// get the program counter of a given cpu.
QEMU_GET_PROGRAM_COUNTER_PROC QEMU_get_program_counter;

QEMU_INCREMENT_DEBUG_STAT_PROC QEMU_increment_debug_stat;

// convert a logical address to a physical address.
QEMU_LOGICAL_TO_PHYSICAL_PROC QEMU_logical_to_physical;

QEMU_BREAK_SIMULATION_PROC QEMU_break_simulation;

QEMU_IS_STOPPED_PROC QEMU_is_stopped;


QEMU_SET_SIMULATION_TIME_PROC QEMU_getSimulationTime;
QEMU_GET_SIMULATION_TIME_PROC QEMU_setSimulationTime;


// dummy function at the moment. should flush the translation cache.
QEMU_FLUSH_ALL_CACHES_PROC QEMU_flush_all_caches;

// determine the memory operation type by the transaction struct.
//[???]I assume return true if it is data, false otherwise
QEMU_MEM_OP_IS_DATA_PROC QEMU_mem_op_is_data;

//[???]I assume return true if it is write, false otherwise
QEMU_MEM_OP_IS_WRITE_PROC QEMU_mem_op_is_write;

//[???]I assume return true if it is read, false otherwise
QEMU_MEM_OP_IS_READ_PROC QEMU_mem_op_is_read;

QEMU_INSTRUCTION_HANDLE_INTERRUPT_PROC QEMU_instruction_handle_interrupt;
QEMU_GET_PENDING_EXCEPTION_PROC QEMU_get_pending_exception;
QEMU_ADVANCE_PROC QEMU_advance;
QEMU_GET_OBJECT_PROC QEMU_get_object;

//NOOSHIN: begin
QEMU_CPU_EXEC_PROC QEMU_cpu_exec_proc;
//NOOSHIN: end

QEMU_IS_IN_SIMULATION_PROC QEMU_is_in_simulation;
QEMU_TOGGLE_SIMULATION_PROC QEMU_toggle_simulation;
QEMU_FLUSH_TB_CACHE_PROC QEMU_flush_tb_cache;

// insert a callback specific for the given cpu or -1 for a generic callback
QEMU_INSERT_CALLBACK_PROC QEMU_insert_callback;

// delete a callback specific for the given cpu or -1 for a generic callback
QEMU_DELETE_CALLBACK_PROC QEMU_delete_callback;

QEMU_GET_INSTRUCTION_COUNT_PROC QEMU_get_instruction_count;

} QFLEX_API_Interface_Hooks_t;

typedef void* (*QMP)(const char *, void*);

void QFLEX_API_get_interface_hooks(QFLEX_API_Interface_Hooks_t* hooks);
void QFLEX_API_set_interface_hooks( const QFLEX_API_Interface_Hooks_t* hooks );

#endif
