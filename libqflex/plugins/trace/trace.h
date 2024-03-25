#ifndef LIBQFLEX_TRACE_H
#define LIBQFLEX_TRACE_H


#include "qemu/osdep.h"
#include "middleware/libqflex/libqflex-legacy-api.h"

typedef struct mem_access {
    uint8_t is_load    :1 ;
    uint8_t is_store   :1 ;
    uint8_t is_vector  :1 ;
    uint8_t is_signed  :1 ;
    uint8_t is_pair    :1 ;
    uint8_t is_atomic  :1 ;

    size_t size;
    uint32_t accesses;
};

typedef struct
{
    size_t          const   insn_size;
    char const *    const   disas_str; //! Super bad, the string my be overwritten in the futur
    uint32_t        const   opcode;
    uint32_t        const   exception_lvl;
    translation_type_t      type;

    logical_address_t const   target_pc_va;
    physical_address_t const  target_pc_ha;  // Require qemu_plugin_get_gpaddr addition to the qemu-plugin.h api, really needed ??
    physical_address_t const  host_pc_ha;

} trace_insn_t;


void qemu_plugin_trace_init(void);

#endif