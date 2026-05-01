# CLAUDE.md — qemu/middleware/

This is a **sub-submodule** of the QFlex simulator. The qflex root is at `../..`; its [CLAUDE.md](../../CLAUDE.md) describes the four-phase pipeline. The parent submodule (the timing QEMU fork) is at `..`; its [CLAUDE.md](../CLAUDE.md) describes how this directory is wired in. This file documents only what's specific to *qemu/middleware/* itself.

## What this submodule is

The **shim that brokers the QEMU↔Flexus connection**. It's a meson subproject embedded into the timing QEMU build via `subdir('middleware')` (called from the parent's [meson.build:3389](../meson.build#L3389) when both `--enable-libqflex` and `--enable-snapvm-external` are set).

Without this directory, [../](.. ) would just be a slow QEMU; this glues it to [../../flexus/](../../flexus/).

## Role in QFlex

**Phase 4 (timing simulation) only.** During QEMU startup (called from [../softmmu/vl.c:3732,3737](../softmmu/vl.c#L3732)):

1. `libqflex_init()` initialises the libqflex side, then calls `libqflex_flexus_init()` which `dlopen`s the Flexus shared object ([libqflex/libqflex-module.c:111](libqflex/libqflex-module.c#L111)) and resolves the `flexus_init` symbol ([:118](libqflex/libqflex-module.c#L118)).
2. It populates a `QEMU_API_t` struct of function pointers (so Flexus can call back into QEMU) and hands it to Flexus, which returns its own `FLEXUS_API_t` struct.
3. From then on, simulation alternates between Flexus calling `cpu_exec`/`tick` on QEMU and QEMU calling `start`/`pause`/`trace_mem` on Flexus.
4. `snapvm_init()` ([snapvm-external/snapvm-external-module.c:56](snapvm-external/snapvm-external-module.c#L56)) wires up the file-based external snapshot mechanism so the timing phase can ingest FW checkpoints from [../../WormCacheQFlex/](../../WormCacheQFlex/).

## Layout

| Path | Purpose |
|---|---|
| `libqflex/` | ARM CPU state wrapper + bidirectional Flexus IPC API (~14 .c/.h files). Wired into meson's `arm_ss` target. Public API in [libqflex/libqflex-legacy-api.h](libqflex/libqflex-legacy-api.h). Module entry in [libqflex/libqflex-module.c](libqflex/libqflex-module.c). |
| `snapvm-external/` | File-based external snapshot save/load (~4 .c files). Wired into `system_ss`. Module entry in [snapvm-external/snapvm-external-module.c](snapvm-external/snapvm-external-module.c). |
| `meson.build` | Top-level wiring. References `arm_ss` / `system_ss` from the parent QEMU build. |
| `trace-events`, `trace.h` | Placeholders. |

There is **no README** here; architecture lives only in header comments.

## Public interface (the heart of this submodule)

All in [libqflex/libqflex-legacy-api.h](libqflex/libqflex-legacy-api.h):

### `QEMU_API_t` — what Flexus calls into QEMU

Function pointers Flexus uses to introspect/drive guest state. Roughly:

- `read_register`, `read_sys_register` — register file access.
- `translate_va2pa` — page table walk.
- `get_pc`, `has_irq` — control-flow / interrupt state.
- `cpu_exec` — advance execution (calls `libqflex_advance` → `libqflex_step`).
- `tick` — per-cycle callback from Flexus; lets QEMU advance virtual time.
- `can_stop`, `is_busy`, `stop` — lifecycle / synchronisation.
- `get_mem` — physical memory read.
- `disassembly` — instruction decode for tracing.

### `FLEXUS_API_t` — what QEMU calls into Flexus

Function pointers the middleware fills in after `dlopen`:

- `start`, `stop` — lifecycle.
- `qmp` — forward QMP commands (e.g. `QMP_FLEXUS_SETSTATINTERVAL`, `QMP_FLEXUS_WRITEMEASUREMENT`; full list in [../../flexus/core/qemu/api.h](../../flexus/core/qemu/api.h)).
- `pause`, `resume`, `is_paused` — flow control.
- `trace_mem` — memory-access tracing hook.

### IPC mechanism

**In-process function pointers — no sockets, no shared memory.** Flexus runs inside the QEMU process via `dlopen`. The Flexus shared object must export a `flexus_init` symbol, or [libqflex/libqflex-module.c:118-120](libqflex/libqflex-module.c#L118) will error out.

## `snapvm-external` API

Public functions: `save_snapshot_external()` / `load_snapshot_external()` — checkpoint VM state (CPU + memory) to/from a path. This bypasses QEMU's internal snapshot path (which is block-device-internal) and is what's used to import FW checkpoints from the WormCacheQFlex output into the timing phase.

Initialisation runs from [snapvm-external/snapvm-external-module.c](snapvm-external/snapvm-external-module.c) (module init around line 56, called from `../softmmu/vl.c`).

## Conventions and gotchas

- **`.gitmodules` quirk:** the parent `../.gitmodules` records this submodule's branch as `fix/default_frequency` (under the `libqflex` entry; the second `middleware` entry has only `branch = develop`), but the actually-checked-out HEAD is `feature/multi-node` — many commits ahead of either recorded branch. **Don't blindly `git submodule update --remote`** without checking what you'd be moving to.
- **Two subdirs, two meson targets.** `libqflex` plugs into `arm_ss` (because it touches ARM CPU state); `snapvm-external` plugs into `system_ss` (system-level lifecycle). Mismatch them and the build won't link.
- **No README, sparse comments.** When in doubt, the header `libqflex-legacy-api.h` is the contract; the rest is implementation. Some design notes are inline comments in `libqflex.c` / `libqflex.h`.
- **Flexus must export `flexus_init`** as a C symbol (no name mangling). If you're modifying Flexus's entry point, this is what `dlsym` is looking for.
- The submodule lives on branch `feature/multi-node`. Recent work: paused-state plumbing, idx-end-at-same-virtual-time support, clock+icount integration, libqflex_tick driving timers.

## See also

- [../../CLAUDE.md](../../CLAUDE.md) — qflex root: four-phase pipeline, `ExperimentContext`, build commands.
- [../CLAUDE.md](../CLAUDE.md) — the timing QEMU fork (parent of this directory). Explains how `subdir('middleware')` is wired in and the `CONFIG_LIBQFLEX` / `CONFIG_SNAPVM_EXT` macros.
- [../../flexus/CLAUDE.md](../../flexus/CLAUDE.md) — the timing model loaded by this shim via `dlopen`. The other side of `FLEXUS_API_t` / `QEMU_API_t` is implemented there.
- [../../WormCacheQFlex/CLAUDE.md](../../WormCacheQFlex/CLAUDE.md) — produces the FW checkpoints `snapvm-external` ingests at the start of the timing phase.
- [../../parallel-qemu/CLAUDE.md](../../parallel-qemu/CLAUDE.md) — fast QEMU used in phases 1–3 (does not embed this middleware).
