# Design Log Index

Catalog of design logs. Append entries here when a new log is created or its scope materially changes.

Status meanings:

- **drafted** — design written; open questions, scope or shape may still move.
- **locked** — design frozen; implementation can begin, no further design churn expected.
- **completed** — implemented and verified; log carries an Implementation Results section.

| #   | Title | Status | Description |
| --- | --- | --- | --- |
| 001 | [Four-channel tester: plan and build order](001-plan.md) | locked | 4 channels: LM324 CC sink up to 1 A, TP4056 via relay module, NTC per cell, 3 buttons + 3 screens, bad-cell rules. Nanit on USB from the 5V bus, Li-Po backup, NO POWER state. `docs/plan.html` + `docs/build.html` (9 soldering stages with checks). Firmware not started. |
| 002 | [Firmware architecture](002-firmware-architecture.md) | drafted | Core logic in hardware-free `lib/tester_core` (native Unity tests), thin Arduino layer; 1 s channel tick; NO POWER via 10k/10k bus divider on A7; service mode for build-stage checks. |
| 003 | [clangd: default env for the compile database](003-clangd-default-env.md) | completed | `Arduino.h not found` in the IDE: compile db was exported for `native`. `default_envs = nanit_pro`; generated compile dbs untracked. |
| 004 | [Phased build](004-phased-build.md) | drafted | Build one stage, flash that phase branch (`phase_1`…`phase_9`), check, continue. Phase 1 = power + bus + A7 sense + Nanit USB. |
