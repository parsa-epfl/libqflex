
# Add libqflex to the ARM target to access CPU register
arm_ss.add(when: middleware_dep['libqflex'], if_true: files(
    'libqflex/libqflex-module.c',
    'libqflex/libqflex.c'
))

# Add pulgins to the utils target to keep coherence with working version
# of the trace plugin
specific_ss.add(when: middleware_dep['libqflex'], if_true: files(
    'libqflex/plugins/trace/trace.c',
))

# Add snapshot related file to the system target to access other snapshot
# library and block file function/structure
system_ss.add(when: middleware_dep['savevm-external'], if_true: files(
    'savevm-external/snapvm-external-module.c',
    'savevm-external/savevm-external.c',
    'savevm-external/loadvm-external.c',
    'savevm-external/snapvm-qmp-cmds.c',
    'savevm-external/snapvm-hmp-cmds.c',
))
