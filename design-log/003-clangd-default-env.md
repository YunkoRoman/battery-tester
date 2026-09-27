# 003 — clangd: default env for the compile database

## Background

The project has two PlatformIO envs since 002: `nanit_pro` (AVR firmware) and `native` (host Unity tests). The prototype already hit PlatformIO's IDE export rebuilding `compile_commands.json` for whichever env it picks (prototype log 001, Q6).

## Problem

In Antigravity, clangd reported `'Arduino.h' file not found` in `src/*.cpp`. The builds themselves were fine.

## Questions and Answers

- **Q1 — Cause?** `compile_commands.json` (root and `.cache/clangd/`, the latter used via `.vscode/settings.json --compile-commands-dir`) had been exported for `native`: compiler `/usr/bin/g++`, no `framework-arduino-avr/cores/arduino` include. With no `default_envs`, the IDE export picked `native`.
- **Q2 — Fix?** `[platformio] default_envs = nanit_pro`. Verified: `pio run -t compiledb` (no `-e`) now processes `nanit_pro`; `src/main.cpp` entry uses `avr-g++` with the Arduino core include; `clangd --check=src/main.cpp` has no diagnostics.
- **Q3 — Test files?** `test/**` have no entry in the `nanit_pro` database, so clangd can't resolve `unity.h` there. Unity is only materialised by PlatformIO during `pio test`, so there is no stable include path to add. IntelliSense-only; `pio test -e native` is unaffected.
- **Q4 — Commit the databases?** No. They are machine-generated with absolute paths — now in `.gitignore` and untracked (was a deferred minor from the firmware review).

## Design

`platformio.ini` gets a `[platformio]` section with `default_envs = nanit_pro` and a comment why. `.gitignore` adds `compile_commands.json` and `.cache`.

## Trade-offs

- Test sources lose IntelliSense for Unity macros; the alternative (switching the export to `native`) would break every `src/` file instead.

## Implementation Results

- `.cache/clangd/compile_commands.json` regenerated for `nanit_pro`; clangd check on `src/main.cpp`, `src/display.cpp`, `src/board_io.cpp`, `lib/tester_core/src/channel.cpp`: 0 diagnostics.
- Nothing flashed.
