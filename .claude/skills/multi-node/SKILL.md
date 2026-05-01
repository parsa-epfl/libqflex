---
name: multi-node
description: QFlex multi-node from the qemu/middleware perspective — owner of the pause handshake (pause/resume/is_paused in FLEXUS_API_t) that PDES uses to halt Flexus, owner of the Flexus tick → QEMU virtual-time path, and snapvm-external for per-node CPU+memory checkpoint. Use when the user asks about how PDES halts Flexus, the FLEXUS_API_t pause API, the libqflex tick callback, snapvm-external, or how multi-node coordination flows between QEMU and Flexus.
---

This skill's content lives in `MULTI_NODE.md` next to this directory's `CLAUDE.md`. Read it now: [../../../MULTI_NODE.md](../../../MULTI_NODE.md).

That doc covers the pause handshake, the virtual-time path, and `snapvm-external`'s role in distributed snapshots, with cross-references to each sibling `MULTI_NODE.md`.
