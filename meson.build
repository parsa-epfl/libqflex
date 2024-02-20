
arm_ss.add(when: middleware_dep['libqflex'], if_true: files(
    'libqflex/libqflex-module.c',
    'libqflex/libqflex.c'
))

system_ss.add(when: middleware_dep['savevm-external'], if_true: files(
    'savevm-external/savevm-external.c',
))