# 012 — Nanit battery % on the TFT

## Background

The tester screen shows the cell under test (V, mA, mAh, time) but not the charge left in Nanit's own Li-Po. A long test (~2.7h at ~1A, see 011) could run the controller battery flat mid-test.

## Problem

Show the remaining charge of Nanit's own battery as a percentage (not volts) on the TFT and in the Serial log.

## Questions and Answers

- **Q1 — Source?** NanitLib `getBatteryVoltage()` reads `BATTERY_PIN` (69 / A15) against the 5V AVCC.
- **Q2 — Why not NanitLib `getBatteryPower()`?** It has two bugs. Segment 7 (3.68–3.8V) interpolates from 3.88V, so the value jumps 22% → ~17% at 3.8V. Its strict `>`/`<` comparisons return 0% when the voltage lands exactly on a segment boundary. The firmware reuses the same 12-point curve with a table lookup instead.
- **Q3 — Where on screen?** Bottom row, y = 119, textSize 1 (8 px), below Time. Label "Nanit", value e.g. `73%`.

## Design

- `readNanitPercent()` in `src/battery_tester.cpp`: `NANIT_CURVE_V[]` / `NANIT_CURVE_P[]` (4.24V = 100% … 2.5V = 0%), linear interpolation, rounded to `uint8_t`.
- `displayUpdate(..., uint8_t nanitPercent)`: new `Y_NANIT` field, redrawn in place like the others.
- Serial: `... | Nanit: 73%`.

## Trade-offs

- Voltage-based %, read under the board's own load, so it is approximate. It is good enough to spot a flat controller battery.
- When Nanit runs from USB, A15 shows the Li-Po/charger voltage, not the USB supply.

## Implementation Results

- `pio run -e nanit_pro` builds clean (RAM 21.5%, Flash 12.1%). Not yet checked on hardware.
