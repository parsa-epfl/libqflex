Add this line to qflex/qemu/hmp-commands.hx

```
ERST
    {
          .name       = "flexus-start-periodic-ckpt",
          .args_type  = "dirname:F? checkpoint_interval:i? sample_size:i?",
          .params     = "dirname checkpoint_interval sample_size",
          .help       = "Start periodic checkpoint generation with optional checkpoint_interval and sample_size",
          .cmd        =  hmp_flexus_start_periodic_ckpt
    },
SRST
 ``flexus-start-periodic-ckpt`` *tag*
 Qflex specific, save uArch state as checkpoints on the disk.
 The tag is a directory the checkpoint will be save to.
```
