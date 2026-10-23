# 013 — Measurement check with a multimeter; V_REF = 5.02

## Background

After the 3×10Ω load (011) was soldered, the user ran a step-by-step multimeter check (5V rail, no-load and under-load voltage, R5 / GND / D–S / G–S drops, current via V_R1).

## Problem

Confirm that the A0 voltage and the computed current match reality, and fix the constants if they don't.

## Questions and Answers

- **Q1 — Results (2026-09-28)?**
  - 5V rail: 5.02V.
  - No load: meter 4.04V, Serial 4.04 / 4.03V. OK.
  - Under load, holder lugs: meter 3.96V, TFT 3.88V. **0.08V low.**
  - Across R5: 0V. OK.
  - Cell (−) to Nanit GND under load: **0.05V.** Should be ~0.
  - D–S: 0.04V; G–S: 5.02V. MOSFET fully on.
  - V_R1 = 3.78V, so 3.78 / 3.2 ≈ 1.18A; TFT showed 1142 mA.
- **Q2 — Cause of the 0.08V?** About 0.05V is load current on the sense-GND path: the sense GND is joined to the power return somewhere other than the holder (−) lug, so Nanit GND sits above the cell (−) (the same class of fault as 008). The rest (~0.016V) is V_REF 5.00 vs 5.02. The no-load reading is correct because without current there is no drop.
- **Q3 — Why TFT mA is ~3% low?** It is computed from the low A0 voltage. It should come right once the GND fault is fixed; recalibrate R_LOAD afterwards.

## Design

- `V_REF = 5.02` (measured).
- Rewire: the sense-GND wire goes straight from the holder (−) lug to Nanit GND, separate from the thick return wire. R5 / R4 connect directly to the holder (+) lug.
- `R_LOAD` unchanged (3.33) until the re-check after rewiring.

## Trade-offs

- V_REF is measured with the meter's ~0.5% accuracy, which is good enough for 10-bit ADC readings.

## Implementation Results

- `V_REF` 5.0 → 5.02; builds clean. Rewiring and re-check pending.
