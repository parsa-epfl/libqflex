#include <stdint.h>
#include <stddef.h>

typedef enum {} branch_type_t;
typedef enum {} translation_type_t;

typedef struct 
{
    size_t target_va_pc;
    size_t target_pa_pc; // Require qemu_plugin_get_gpaddr addition to the qemu-plugin.h api, really needed ??

    uint32_t opcode;
    size_t instruction_bytes_size;

    bool is_userland; // Require qemu_plugin_is_userland addition to the qemu-plugin.h api, really needed ??

    translation_type_t type;



} transaction_state_t;

struct memory_transaction_state
{
    transaction_state_t transaction;

    uint64_t va_page;
    uint64_t pa_page;

    bool is_io;
    bool is_store;
};

struct instr_transaction_state
{
    transaction_state_t transaction;
    branch_type_t branch_type;
};

// Define a trace callback
//! Should not use void* but a defined type
// typedef void (*qflex_trace_cb_t) (void* userdata, void* cb);