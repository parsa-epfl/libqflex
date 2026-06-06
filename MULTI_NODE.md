# MULTI_NODE.md — qemu/middleware/

This file documents what the QEMU↔Flexus middleware contributes to QFlex multi-node and how it relies on the other submodules. The cross-cutting overview is in [../../MULTI_NODE.md](../../MULTI_NODE.md). Read that first if you haven't.

For this submodule's general context (role, build, layout, the `FLEXUS_API_t` / `QEMU_API_t` API) see [CLAUDE.md](CLAUDE.md).

## What this submodule contributes to PDES

Two things, both load-bearing for multi-node in the timing phase:

### 1. The pause handshake

`pause`, `resume`, and `is_paused` in **`FLEXUS_API_t`** ([libqflex/libqflex-legacy-api.h](libqflex/libqflex-legacy-api.h)) are the actual mechanism PDES uses to halt the timing model when this node has run past a sync barrier. The flow:

```
PDESEngine / WWT in QEMU         middleware                        Flexus
  decides "must wait"     →   FLEXUS_API_t.pause()        →   stop ticking
  …
  neighbour caught up     →   FLEXUS_API_t.resume()       →   resume ticking
```

There's a corresponding `PauseStatusCallBack` registered with the engine ([../../parallel-qemu/include/net/pdes-engine.h:33](../../parallel-qemu/include/net/pdes-engine.h#L33), `is_waiting_for_quanta`) so PDES knows when this node's tick path is currently blocked and can plan accordingly.

Without this handshake, in the timing phase Flexus would race ahead while the engine waits for a neighbour, and virtual time would be wrong by the time the neighbour caught up.

### 2. The virtual-time path

In the timing phase, PDES's notion of virtual time is **what Flexus says it is**, not what icount says. The chain:

```
Flexus cycle count
  → FLEXUS_API_t.tick callback (from Flexus)
  → middleware updates QEMU's notion of virtual time
  → PDES reads virtual time
  → WWT decides whether to send/sync/pause
```

Without this glue, PDES would see icount in the timing fork (which would defeat the point of running a timing simulator at all). The recent `feature/multi-node` branch work — "made libqflex_tick update timers", "made clock work with icount", "support for idx end at same virtual time" — is exactly this path being made reliable.

### 3. Per-node CPU+memory checkpoint

`snapvm-external/` exposes file-based `save_snapshot_external()` / `load_snapshot_external()` (~lines 50–85 in [snapvm-external/snapvm-external-module.c](snapvm-external/snapvm-external-module.c)). During distributed snapshots, this is what writes the per-node CPU+memory state — separately from the in-flight-message JSON in [../../parallel-qemu/net/pdes-checkpoint.c](../../parallel-qemu/net/pdes-checkpoint.c). Together they form a coherent distributed checkpoint at a quantum-aligned virtual time.

## How this submodule uses the other submodules

- **[../](../)** (the timing QEMU fork) — this directory is embedded into that build via `subdir('middleware')` ([../meson.build:3389](../meson.build#L3389)) when `--enable-libqflex` and `--enable-snapvm-external` are set. The middleware is initialised from `softmmu/vl.c:3732` (`libqflex_init`) and `:3737` (`snapvm_init`). PDES code (`net/pdes-*.c`) lives in the parent fork, not here — the middleware is the bridge, not the engine.

- **[../../flexus/](../../flexus/)** — `dlopen`'d from [libqflex/libqflex-module.c:111](libqflex/libqflex-module.c#L111); `flexus_init` symbol resolved at [:118](libqflex/libqflex-module.c#L118). Flexus must implement `pause`, `resume`, `is_paused`, and the cycle-driven `tick` for multi-node coordination to work — without those, the pause handshake and virtual-time path both break.

- **[../../parallel-qemu/](../../parallel-qemu/)** — not used at runtime (different binary). But the PDES engine in *this* fork (a copy of parallel-qemu's `net/pdes-*.c`) is what calls into this middleware. So the engine's expectations on `tick` cadence and `pause` timing must match what the middleware delivers.

- **[../../WormCacheQFlex/](../../WormCacheQFlex/)** — not used at runtime. But the FW checkpoints WormCache produced (per-sampling-unit JSON+zstd) are loaded through `snapvm-external` at the start of each timing-phase sampling unit.

## See also

- [../../MULTI_NODE.md](../../MULTI_NODE.md) — the comprehensive cross-cutting overview.
- [../MULTI_NODE.md](../MULTI_NODE.md) — the timing QEMU fork that embeds this middleware.
- [../../flexus/MULTI_NODE.md](../../flexus/MULTI_NODE.md) — the cycle-count clock and the implementer of `pause`/`resume`/`is_paused` on the Flexus side.
- [../../parallel-qemu/MULTI_NODE.md](../../parallel-qemu/MULTI_NODE.md) — owner of the canonical PDES engine that drives the pause / tick interactions.
- [../../WormCacheQFlex/MULTI_NODE.md](../../WormCacheQFlex/MULTI_NODE.md) — produces the FW checkpoints `snapvm-external` ingests.
