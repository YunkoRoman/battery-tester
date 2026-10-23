# Project Instructions

## Project Overview

18650 battery capacity tester on a Nanit Pro (ATmega2560) board via NanitLib. Currently a 1-channel MOSFET discharge prototype; 4-channel scale-out planned. Firmware in `src/battery_tester.cpp`, built/uploaded via PlatformIO (`pioarduino` extension) in Antigravity, env `nanit_pro`.

See `design-log/001-starter.md` for the full circuit, pin mapping, and toolchain setup history.

## Design Log

After every change that touches the circuit, pin assignments, firmware architecture, or toolchain setup — component value changes, new channels, new libraries, build/IDE config fixes — update `design-log/`:

1. Create a new numbered log file (`NNN-short-slug.md`) following the format of `001-starter.md`: Background, Problem, Questions and Answers, Design, Trade-offs, Implementation Results.
2. Append a row to `design-log/index.md` with the log number, title (linked), status, and a one-line description.

What counts as loggable:
- Circuit/schematic changes (component values, new channels, new signal pins)
- Firmware behavior changes (cutoff logic, timing, new measurements)
- Toolchain/build config fixes (PlatformIO, clangd, library pinning)
- New dependencies or board/library version pins

What does not require a log entry:
- Typo fixes, comment-only changes
- Cosmetic formatting
