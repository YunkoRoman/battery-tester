# 006 — No-battery detection (R4 pull-down + NO BATTERY / BATT REMOVED states)

## Background

A0 (P10_2) connects only to Battery+. With no cell inserted, the only path from that node is R1 → drain of an off MOSFET (µA leakage), so A0 floats and the ADC reads noise anywhere in 0–5V. Since 004/005 the cutoff is latched: one noisy sample > 3.0V opened the gate, R1+MOSFET then pulled A0 to ~0V, 5 low samples later the test latched DONE with ~0.1 mAh — and a real cell inserted afterwards would not start until RESET.

## Problem

1. Give A0 a defined level with no cell present.
2. Show "no battery" explicitly instead of noise or a fake DONE.
3. Don't start on a single sample; detect a cell pulled mid-test.

## Questions and Answers

- **Q1 — Pull-down value?** *(Changed to 10kΩ in 007 — no 100kΩ on hand.)* 100kΩ (R4, P10_2 → GND). Drains ≈ 42µA from a 4.2V cell — ~0.34 mAh over an 8h test, negligible vs ~3000 mAh, and not counted by the V/R_LOAD integration. Low enough to dominate MOSFET leakage and ADC input bias; high enough to be no load on the cell.
- **Q2 — "No battery" threshold?** `NO_BATTERY_VOLTAGE = 0.5V`. A healthy or even deeply discharged cell sits far above it; with R4 an empty slot reads ~0V.
- **Q3 — Start confirmation?** `START_CONFIRM_SAMPLES = 3` consecutive samples > 3.0V (3s). Also covers contact bounce while inserting a cell.
- **Q4 — Cell removed mid-test?** Under load a real cell never reads < 0.5V, so one such sample while discharging = removed. Gate LOW, `testAborted = true`, latched until RESET like DONE — a partial result must not be continued with a different cell. Capacity so far stays on screen.

## Design

Firmware (`battery_tester.cpp`):

- `batteryPresent = voltage >= NO_BATTERY_VOLTAGE`.
- Not started: `highVoltageCount` counts consecutive samples > cutoff; start at 3.
- Discharging and `!batteryPresent` → stop, `testAborted`.
- Discharging otherwise → 5-sample cutoff from 005.
- Display state priority: Discharging → Removed → Done → NoBattery → Idle.

Display (`display.h/.cpp`): `TesterState` gains `NoBattery` ("NO BATTERY", red) and `Removed` ("BATT REMOVED", red).

Schematic (`scratch_schematic.html`, rev 03): R4 100kΩ between the Battery+/A0 node and GND; BOM, pin map, assembly steps and firmware state table updated.

## Trade-offs

- Removal detection on one sample: a contact glitch exactly at the 1s sample instant would abort a long test. Accepted — a glitch that drops a loaded cell to < 0.5V is itself a wiring fault worth seeing.
- 3s start delay after inserting a cell — irrelevant for multi-hour tests.

## Implementation Results

- `pio run -e nanit_pro` builds clean (RAM 20.0%, Flash 11.8%).
- Schematic rev 03 rendered and checked in headless Chrome.
- Not yet verified on hardware (R4 not yet fitted).
