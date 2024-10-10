#ifndef LIBQFLEX_TRACE_H
#define LIBQFLEX_TRACE_H

#include "qemu/osdep.h"
#include "middleware/libqflex/libqflex-legacy-api.h"

struct mem_access {
        uint8_t is_load : 1;
        uint8_t is_store : 1;
        uint8_t is_vector : 1;
        uint8_t is_signed : 1;
        uint8_t is_pair : 1;
        uint8_t is_atomic : 1;

        size_t size;
        uint32_t accesses;
};

typedef struct {
        size_t byte_size;
        char const
            *disas_str; //! Super bad, the string my be overwritten in the futur
        uint32_t opcode;
        uint32_t exception_lvl;
        mem_op_type_t type;

        logical_address_t target_pc_va;

} trace_insn_t;

void libqflex_trace_init(void);

bool decode_armv8_mem_opcode(struct mem_access *, uint32_t);

bool decode_armv8_branch_opcode(branch_type_t *, uint32_t);

#endif
