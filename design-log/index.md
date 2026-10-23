# Design Log Index

Catalog of design logs. Append entries here when a new log is created or its scope materially changes.

Status meanings:

- **drafted** — design written; open questions, scope or shape may still move.
- **locked** — design frozen; implementation can begin, no further design churn expected.
- **completed** — implemented and verified; log carries an Implementation Results section.

| #   | Title | Status | Description |
| --- | --- | --- | --- |
| 001 | [Starter: 1-channel MOSFET discharge tester on Nanit Pro](001-starter.md) | drafted | Pivoted from breadboard build to the physical Nanit Pro board (NanitLib pins P5_1/P10_2, GND common). PlatformIO + pioarduino IDE toolchain in Antigravity, arduino-cli fallback. NanitLib + FastLed_Neopixel_Nanit cloned from GitHub (not on registry), FastLED pinned to 3.6.0 for compatibility. `.cpp` sketches split into separate PlatformIO projects per sketch to avoid PlatformIO's `.ino`-merge and compile_commands.json overwrite bugs. `.clangd` Remove-filter strips a stray ESP32 include PlatformIO's IDE export injects. Load resistor bank initially set at 4×10Ω/10W parallel (superseded by 002). hello_world.cpp flashed and confirmed working on the TFT; discharge circuit not yet physically assembled.
| 002 | [Load resistor: back to single 10Ω/10W](002-single-10ohm-load.md) | locked | Replaced 4×10Ω parallel bank (2.5Ω) with a single 10Ω/10W load: I_max≈0.42A, P≈1.76W, ~8h per 3000mAh cell. `R_LOAD = 10.0`, builds clean. Schematic updated in 004. |
| 003 | [TFT display module](003-tft-display-module.md) | locked | Separate `src/display.{h,cpp}` module shows status, V, mA, mAh and discharge time on the built-in 160x128 ST7735. Flicker-free in-place redraw. Builds clean; not yet checked on hardware. |
| 004 | [Cutoff latch + schematic refresh](004-cutoff-latch-and-schematic.md) | locked | `testFinished` latch stops discharge from restarting after cutoff when the cell's voltage recovers; restart via board RESET. New schematic Artifact (single 10Ω) from `docs/schematic.html`. |
| 005 | [Cutoff debounce (N consecutive samples)](005-cutoff-debounce.md) | locked | Cutoff requires 5 consecutive samples ≤ 3.0V (`CUTOFF_CONFIRM_SAMPLES`) so one noisy ADC reading can't end the latched test. `scratch_schematic.html` rev 02 updated to single 10Ω + firmware states. |
| 006 | [No-battery detection](006-no-battery-detection.md) | locked | R4 pull-down (100kΩ → 10kΩ in 007) on A0 so an empty slot reads 0V, not ADC noise. New NO BATTERY / BATT REMOVED states, 3-sample start confirm, abort + latch if the cell is pulled mid-test. Schematic rev 03. |
| 007 | [R4 = 10kΩ; single schematic source](007-r4-10k-single-schematic.md) | locked | R4 changed to 10kΩ (no 100kΩ on hand); its ~0.4 mA added to the mAh integration. Published schematic Artifact and `docs/schematic.html` deleted — `scratch_schematic.html` (rev 04) is the only schematic. |
| 008 | [Separate sense wiring (Kelvin)](008-kelvin-sense-wiring.md) | completed | First hardware run: cell 3.66V under load but A0 read ~2V → instant cutoff. Likely drop in shared breadboard load path. Fix: soldered power loop + separate sense wires from the holder to P10_2/GND. Rewired 2026-09-25 — works. |
| 009 | [R5: series resistor on A0](009-a0-series-resistor.md) | locked | 10kΩ in series before A0 (R4 stays cell-side) so the tested cell can't power Nanit through the pin's ESD diode when the switch is off. Schematic rev 06; not yet fitted. |
| 010 | [Four-channel tester with charging (plan)](010-four-channel-plan.md) | drafted | 4 channels: LM324 CC sink up to 1A, TP4056 via relay module, NTC per cell, 3 buttons + 3 screens, bad-cell rules, BOM. Moved to `../battery-tester-4ch`. |
| 011 | [Load: 3×10Ω/10W parallel (~1A)](011-three-10ohm-parallel-1a.md) | locked | R1 = 3×10Ω parallel = 3.33Ω: 1.26A @ 4.2V … 0.9A @ 3.0V, ~2.7h per 3000mAh. `R_LOAD` = 3.25 after calibration (U_R1 / 3.2Ω). Schematic rev 07. |
| 012 | [Nanit battery % on the TFT](012-nanit-battery-percent.md) | locked | Bottom TFT row + Serial show Nanit Li-Po % from A15. Own curve lookup instead of NanitLib `getBatteryPower()` (segment-7 bug, 0% at exact boundaries). Builds clean. |
| 013 | [Measurement check; V_REF = 5.02](013-measurement-check.md) | locked | Meter check: no-load OK, under load TFT 0.08V low. 0.05V of that is load current on the sense-GND path (rewire to the holder − lug). `V_REF = 5.02` measured. R_LOAD recal after rewiring. |
| 014 | [A0 averaging (64 samples / 20 ms)](014-adc-averaging.md) | locked | `readBatteryVoltage()` averages 64 reads over one 50 Hz period and discards the first read after the A15→A0 mux switch. Wiring check (013) passed: 3 mV GND, 5 mV (+). Builds clean. |
