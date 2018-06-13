#ifndef __LIBQEMUFLEX_FLEXUS_PROXY_HPP__
#define __LIBQEMUFLEX_FLEXUS_PROXY_HPP__
#include "api.h"

typedef struct simulator_obj simulator_obj_t;

simulator_obj_t* simulator_load( const char* path );

void simulator_unload( simulator_obj_t* obj );

typedef void (*SIMULATOR_INIT_PROC)(QFLEX_API_Interface_Hooks_t*);
extern SIMULATOR_INIT_PROC simulator_init;

typedef void (*SIMULATOR_PREPARE_PROC)(void);
extern SIMULATOR_PREPARE_PROC simulator_prepare;

typedef void (*SIMULATOR_DEINIT_PROC)(void);
extern SIMULATOR_DEINIT_PROC simulator_deinit;

typedef void (*SIMULATOR_START_PROC)(void);
extern SIMULATOR_START_PROC simulator_start;

typedef void (*SIMULATOR_BIND_QMP_PROC)(qmp_flexus_cmd_t, const char*);
extern SIMULATOR_BIND_QMP_PROC simulator_qmp;

#endif /* __LIBQEMUFLEX_FLEXUS_PROXY_HPP__ */
