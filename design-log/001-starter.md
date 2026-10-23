# 001 — Starter: 1-channel MOSFET discharge tester on Nanit Pro

## Background

Hobby project: measure real discharge capacity (mAh) of 18650 Li-ion cells. Originally scoped as a 4-channel breadboard build on a bare Nanit Pro (ATmega2560) board with individually-sourced parts (BOM: IRL540N MOSFETs, 10Ω/10W ceramic resistors, 220Ω/10kΩ gate resistors, 18650 holders). Mid-build, pivoted to a physical Nanit Pro robot dev board already on hand (round board, built-in 1.8" TFT, built-in Li-Po, NanitLib) instead of a plain breadboard build, since it already has a screen and its own battery for the *controller*, and exposes 12 edge connectors (P1–P12, 4 pins each: GND / raw-battery-3.7V / 4 independent Signal pins) for wiring external circuits.

Current stage: 1-channel prototype only. 4-channel scaling deferred.

## Problem

Build a firmware + circuit that:

- Discharges one 18650 cell through a fixed load, controlled by an N-MOSFET gated from an Arduino digital pin.
- Reads battery voltage during discharge to compute instantaneous current and integrate mAh over time.
- Stops discharge automatically at a safe cutoff voltage (over-discharge protection).
- Runs on the Nanit Pro board (ATmega2560) via its NanitLib pin-naming convention, not raw Arduino pin numbers.
- Builds and uploads from Antigravity (VSCode-fork IDE) rather than the classic Arduino IDE.

## Questions and Answers

- **Q1 — Which board?** Physical Nanit Pro v3.1 (already owned) instead of a fresh breadboard build. Reason: has its own screen + battery for the controller electronics; external circuit only needs the discharge MOSFET/resistor/battery-under-test wired to its edge connectors.
- **Q2 — Which pins carry the discharge signal and voltage sense?** `P5_1` (→ D2, PWM/INT0) drives the MOSFET gate; `P10_2` (→ A0) reads battery voltage. Picked to match the original raw-pin design (D2/A0) 1:1, and because neither P5 nor P10 is in the ⚠️-flagged battery-voltage-power group (P1/P2/P11/P12 run their + rail off the raw board battery, not a concern here since we don't use the + rail at all — only Signal + shared GND).
- **Q3 — IDE?** Antigravity (Google's VSCode fork). No `vscode-arduino` on OpenVSX (Microsoft-only extension) — used `pioarduino.pioarduino-ide` (PlatformIO fork) instead, backed by `arduino-cli` for one-off compiles/sanity checks outside the IDE.
- **Q4 — Where does NanitLib come from?** Not on the PlatformIO registry — cloned directly from `github.com/NanitRobot/NanitLib` (v1.5.2) into `lib/`. Its `library.properties` declares hard deps (Adafruit GFX, Adafruit ST7735/ST7789, FastLED, Servo, Adafruit MPU6050, NonBlockingRTTTL, Adafruit NeoPixel, `FastLed_Neopixel_Nanit`) — the last one is *also* not on the registry, cloned separately from `github.com/kireev7/FastLed_Neopixel_Nanit`.
- **Q5 — FastLED version?** Pinned to `3.6.0`. Latest (3.10.5) breaks `FastLed_Neopixel_Nanit` with an ambiguous `memset` overload (FastLED's own `fl::memset` vs. libc's, both visible via ADL) — a real incompatibility between the custom library (written for older FastLED) and current FastLED, not a bug in our code.
- **Q6 — One PlatformIO project or several sketches in one?** Split into two separate projects after repeated failures: `battery-capacity-tester` (the real `nanit_pro` env) and a standalone `nanit-hello-world` project for the display smoke-test. PlatformIO's `.ino`-merge step and its `build_src_filter` don't compose reliably for multiple `.ino`/multi-env setups in one `src/` — see Trade-offs.
- **Q7 — Resistor load value?** Started at 10Ω/10W (I_max = 0.42A) per the original BOM. User has 4× 10Ω/10W on hand and wants heavier load current, so settled on all 4 in parallel: 2.5Ω equivalent, 40W combined rating, I_max ≈ 1.68A at full charge (4.2V). MOSFET dissipation at that current (I²×Rds(on), Rds(on)≈0.06Ω) ≈ 0.17W — fine without a heatsink.

## Design

### Circuit (1 channel)

```
Battery(+) ──┬── 4× [10Ω/10W] ∥ (=2.5Ω, 40W) ── MOSFET Drain
             └── Nanit P10_2 (A0)              MOSFET Source ── GND (common)
                                                MOSFET Gate ── 220Ω ── Nanit P5_1 (D2)
                                                MOSFET Gate ── 10kΩ ── GND (pull-down)
Battery(−) ── GND (common: also Nanit adapter-port GND, MOSFET source)
```

Full schematic originally published as Artifact 5ec31890-… (no longer available). Current schematic: `scratch_schematic.html` (local file, see 007).

### Repo layout

```
battery-capacity-tester/
├── design-log/            # this log system
├── src/
│   └── battery_tester.cpp # single sketch, .cpp not .ino (see Trade-offs Q6)
├── lib/
│   ├── NanitLib/                  # cloned from NanitRobot/NanitLib
│   └── FastLed_Neopixel_Nanit/    # cloned from kireev7/FastLed_Neopixel_Nanit
├── platformio.ini         # env:nanit_pro — atmelavr / megaatmega2560
├── .clangd                 # strips a stray ESP32 include PlatformIO's IDE
│                            # export injects for any lib declaring architectures=*
└── compile_commands.json / .cache/clangd/compile_commands.json
```

Sibling project `~/Desktop/Projects/nanit-hello-world/` — same lib/platformio.ini shape, single `hello_world.cpp` sketch, used as the first smoke-test (writes "Hello World!" to the built-in 1.8" TFT via the global `tft` object NanitLib exposes after `Nanit_Base_Start()`).

### Pin mapping

| Nanit connector | Signal | Wired to |
|---|---|---|
| P5_1 | D2 (Signal) | R2 (220Ω) → MOSFET Gate |
| P10_2 | A0 (Signal) | Battery+ (direct voltage sense) |
| any | GND | Common: Battery−, MOSFET Source |

### Firmware logic (`battery_tester.cpp`)

1s poll loop: reads `analogRead(A0)` → voltage; if `voltage > CUTOFF_VOLTAGE (3.0V)` and not yet discharging, sets gate HIGH and starts integrating `capacityMah += (V/R_LOAD × 1000) / 3600` per second; drops gate LOW and stops once voltage falls to/below cutoff.

## Trade-offs

- **Physical board reuse vs. fresh breadboard build**: saves a screen + battery + enclosure, but couples the tester to NanitLib's pin macros and its dependency tree (FastLED/Adafruit/etc. all get pulled in for even a 2-pin discharge sketch) — a plain Arduino Mega would need none of that.
- **`.cpp` sketches instead of `.ino`**: PlatformIO's Arduino-framework builder merges *all* `.ino` files in `src/` into one translation unit as a separate pre-build step that **ignores** `build_src_filter` — so two `.ino` sketches in one `src/` silently break (`undefined reference to setup/loop`) no matter how the filter is written. Renaming to `.cpp` (with an explicit `#include <Arduino.h>`) skips that merge step entirely and lets `build_src_filter`/multiple envs work as documented.
- **One project per sketch vs. multiple envs in one project**: even with `.cpp` + `build_src_filter` working for `pio run`, PlatformIO's separate **IDE metadata export** (`compile_commands.json`, used by clangd) rebuilds that file from scratch on every build/upload for *whichever env just ran*, permanently dropping the other env's entries — so clangd flips between "sketch A resolves, sketch B doesn't" after every build. Splitting `hello_world.cpp` into its own PlatformIO project (`nanit-hello-world/`) removes the shared-`compile_commands.json` contention entirely; the cost is duplicated `lib/NanitLib` + `lib/FastLed_Neopixel_Nanit` copies and a duplicated `platformio.ini` dependency block.
- **`.clangd` `Remove:` filter for the ESP32 include**: PlatformIO's compiledb/IDE-export step injects `-I .../framework-arduinoespressif32/cores/esp32` into the compile command for *any* library that declares `architectures=*` (NanitLib does), even though the real AVR build never sees that path — it resolved `<Arduino.h>` to the ESP32 core and broke IntelliSense (`println` overload errors, file-not-found). Fixed at the `.clangd` config level (`CompileFlags.Remove` glob) rather than patching the generated file each time, since the file regenerates on every build.
- **dialout group / usb permissions**: `arduino-cli`/PlatformIO upload needs the user in the `dialout` group; without a fresh login the group change doesn't take effect in the running desktop session, so `sudo chmod 666 /dev/ttyUSB0` was used as a same-session workaround.

## Implementation Results

- 1-channel circuit designed and documented (schematic Artifact above); physical build in progress.
- `battery_tester.cpp` compiles clean under `pio run -e nanit_pro` (AVR toolchain, NanitLib + deps resolved, FastLED pinned to 3.6.0).
- `nanit-hello-world/hello_world.cpp` — first firmware actually flashed to the board; confirmed working (`avrdude: 26226 bytes of flash verified`, "Hello World!" rendered on the TFT).
- Load resistor bank finalized at 4× 10Ω/10W in parallel (2.5Ω, 40W, I_max ≈ 1.68A) — code (`R_LOAD` in `battery_tester.cpp`) and schematic both updated to match.
- Toolchain fully working end-to-end from Antigravity: build, upload (`/dev/ttyUSB0`), serial monitor, and clangd IntelliSense all functional for both projects.
- 2026-09-24: circuit assembled on a breadboard, firmware flashed; first run exposed a sense-wiring problem (see 008). Still to do: first full discharge test run, 4-channel scale-out.
