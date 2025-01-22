
# Add libqflex to the ARM target to access CPU register
arm_ss.add(when: middleware_dep['libqflex'], if_true: files(
    'libqflex/libqflex.c',
    'libqflex/libqflex-module.c',
    'libqflex/libqflex-hmp-cmds.c',
    'libqflex/libqflex-qmp-cmds.c',
))

# Add snapshot related file to the system target to access other snapshot
# library and block file function/structure
system_ss.add(when: middleware_dep['snapvm-external'], if_true: files(
    'snapvm-external/snapvm-external-module.c',
    'snapvm-external/savevm-external.c',
    'snapvm-external/loadvm-external.c',
    'snapvm-external/snapvm-qmp-cmds.c',
    'snapvm-external/snapvm-hmp-cmds.c',
))
