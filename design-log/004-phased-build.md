# 004 — Phased build: one circuit, one firmware, one check

## Background

The complete firmware exists on branch `firmware` (logs 002–003), but none of the 4-channel hardware is built yet. Flashing the full firmware onto a half-built board gives little signal: most screens show empty channels, and a wiring fault in one stage is hard to isolate.

## Problem

Build and verify the tester incrementally: solder one stage, flash firmware that exercises only that stage, check it, move on.

## Questions and Answers

- **Q1 — How are phases stored?** One git branch per phase (`phase_1` … `phase_9`) on `github.com/YunkoRoman/battery-tester`, each starting from the previous phase. `firmware` keeps the complete code.
- **Q2 — Phase 1 scope?** Build stage 1: PSU → switch → fuse → +5V bus, 10k/10k bus divider → A7, USB to Nanit. Firmware: bus voltage, POWER OK / NO POWER via `tc::PowerMonitor` (same hysteresis as the full firmware), Nanit Li-Po V and %.
- **Q3 — Why hold all channel outputs off in phase 1?** Nothing is connected to them yet, but the board may still carry parts of the 1-channel prototype on P5_1 / P10_2; driving every SET / relay / fan / NTC_PWR pin LOW is the safe default.
- **Q4 — Which core code goes into phase 1?** Only `measure`, `calc`, `power`, `settings` (for `kChannels` in `config.h`) and their tests; the rest arrives with the phase that uses it.

## Design

Phase table in `README.md`. `src/main.cpp` on `phase_1`: 1 s loop, ADC ×16 averaging with a discarded first read, TFT via Arduino `Print` (float printing works there; only `snprintf` lacks `%f`), serial CSV `busV,powerOk,liPoV`.

## Trade-offs

- Each phase's `main.cpp` is throw-away scaffolding replaced by later phases; the tested core modules carry over unchanged.
- Branch-per-phase means fixes found in a later phase have to be merged forward, not back.

## Implementation Results

- `phase_1`: `pio run -e nanit_pro` SUCCESS (RAM 19.2 %, Flash 11.1 %), `pio test -e native` 23/23. Not yet flashed.
