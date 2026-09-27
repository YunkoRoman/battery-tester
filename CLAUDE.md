# Project Instructions

## Project Overview

4-channel 18650 battery tester on a Nanit Pro (ATmega2560) board via NanitLib. Each channel: CC discharge 0.1–1.0 A (LM324 + IRL540N + 1Ω shunt), TP4056 charging switched by a relay module, V/I/NTC measurement, full auto cycle (charge → Ri test → discharge → recharge), bad-cell detection. 3 buttons, overview + per-channel + settings screens. Firmware built/uploaded via PlatformIO (`pioarduino` extension) in Antigravity, env `nanit_pro`.

Successor of the 1-channel prototype in `../battery-capacity-tester` (kept unchanged).

- `docs/plan.html` — architecture, pin map, screens, cycle, bad-cell rules, heat, BOM.
- `docs/build.html` — build guide: small schematics in soldering order, each stage with a check.
- `design-log/` — design history.

Power: 14V 35W monitor adapter → master switch → 3A fuse → XL4016 buck set to 5.05 V → 6A fuse → +5V/GND bus, SMBJ5.0A TVS across the bus. Nanit runs from the bus over USB; its own Li-Po (switch ON) is a backup. Master switch off ⇒ relays open, LM324 unpowered, gates pulled low; firmware shows NO POWER and pauses.

## Phases

Built and flashed one circuit at a time: branch `phase_N` = firmware for build stage(s) of that phase (table in `README.md`). Each phase branch starts from the previous one and adds only the code its stage needs; `firmware` holds the complete firmware. Don't flash a phase's firmware onto hardware from a later phase without checking the pin usage.

## Design Log

After every change that touches the circuit, pin assignments, firmware architecture, or toolchain setup — component value changes, new channels, new libraries, build/IDE config fixes — update `design-log/`:

1. Create a new numbered log file (`NNN-short-slug.md`) following the format of `001-plan.md`: Background, Problem, Questions and Answers, Design, Trade-offs, Implementation Results.
2. Append a row to `design-log/index.md` with the log number, title (linked), status, and a one-line description.

What counts as loggable:
- Circuit/schematic changes (component values, new channels, new signal pins)
- Firmware behavior changes (cutoff logic, timing, new measurements)
- Toolchain/build config fixes (PlatformIO, clangd, library pinning)
- New dependencies or board/library version pins

What does not require a log entry:
- Typo fixes, comment-only changes
- Cosmetic formatting
