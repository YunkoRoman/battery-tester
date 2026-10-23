# 014 — A0 averaging (64 samples over ~20 ms)

## Background

The multimeter check (013) confirmed the wiring: sense GND drop 3 mV, (+) lug → R5 5 mV, across R5 0 mV. Under load the TFT still read ~0.04V below the holder and jittered by a few hundredths. A multimeter probe on the high-impedance A0 line made the reading jump 3.74 ↔ 3.68V: the line picks up 50 Hz hum after R5 (10kΩ).

## Problem

A single `analogRead` per second shows ADC noise and mains pickup directly. The cutoff and the mAh integration use that noisy value.

## Questions and Answers

- **Q1 — How many samples?** 64: sum ≤ 64 × 1023 fits easily in `unsigned long`; noise drops by ~8× (√64).
- **Q2 — Why spread over 20 ms?** One full 50 Hz period, so the hum averages out instead of aliasing. Each sample takes ~112 µs `analogRead` + 200 µs gap, so 64 samples ≈ 20 ms, well inside the 1 s loop.
- **Q3 — Channel switch?** `readNanitPercent()` reads A15 right after A0, so the next A0 read follows a mux switch. The first sample after a switch, through the 10kΩ source, is discarded.
- **Q4 — Hardware filter?** Optional: 100 nF from P10_2 to GND at the Nanit end (RC with R5 ≈ 1 ms). Not fitted yet.

## Design

`readBatteryVoltage()`: one dummy `analogRead(VOLTAGE_PIN)`, then 64 reads with `delayMicroseconds(ADC_AVG_GAP_US = 200)`, average → volts with `V_REF`.

## Trade-offs

- It adds ~20 ms of blocking per second, which is harmless for this loop.
- Averaging removes noise but not a fixed offset (meter/ADC accuracy ±0.01–0.02V each).

## Implementation Results

- Builds clean (RAM 21.5%, Flash 12.1%). Hardware re-check pending (TFT stability, 3.1 difference, step 4 current).
