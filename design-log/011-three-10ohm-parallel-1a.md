# 011 — Load: 3×10Ω/10W parallel (~1A discharge)

## Background

Log 002 set the load to a single 10Ω/10W (I ≈ 0.42A max, ~8h per 3000mAh cell). The user has three more 10Ω/10W resistors and wants roughly 1A discharge.

## Problem

Get ~1A discharge current out of the 10Ω/10W resistors on hand, with minimal changes to the working 1-channel prototype.

## Questions and Answers

- **Q1 — Which combination?** Options from 4×10Ω: 3 parallel = 3.33Ω (1.26A @ 4.2V, 1.11A @ 3.7V, 0.90A @ 3.0V, ~1.1A mean); (10+10)‖10‖10 = 4Ω (~0.92A mean); 4 parallel = 2.5Ω (~1.5A mean, too much). Chosen: **3 in parallel**. The 4th resistor stays spare.
- **Q2 — Exactly 1.00A constant?** Not possible with a resistive load: current follows cell voltage. A constant current needs an op-amp CC sink (see 010) or gate PWM. Not needed for this prototype.
- **Q3 — Resistor / MOSFET heat?** Each resistor ≤ 1.76W (same as before), ≤ 5.3W total, so they need spacing. IRL540N: 1.26² × ~0.06Ω ≈ 0.1W, so no heatsink.
- **Q4 — Accuracy?** Current is computed as V_cell / R_LOAD. At 3.33Ω the power-path resistance (wires, contacts, Rds(on), ~0.1–0.2Ω) is no longer negligible: it overstates current by a few %. Fix: calibrate `R_LOAD = V_A0 / I_ammeter` under load.
- **Q5 — Cutoff?** Kept at 3.0V. Sag under ~1A is ~0.1V, so the reported capacity is slightly lower than at 0.4A. That is acceptable for a 1A test.

## Design

```
Battery(+) ──┬── [10Ω ‖ 10Ω ‖ 10Ω = 3.33Ω] ── MOSFET Drain
             └── R5 ── P10_2 (A0)       MOSFET Source ── Battery(−)
```

- Firmware: `R_LOAD = 3.33` (to be replaced by the calibrated value).
- Power loop: AWG20–22, soldered only; return current goes Source → cell (−) directly, not through Nanit GND.
- `scratch_schematic.html` rev 07: R1 = 3×10Ω ‖ in both diagrams, BOM, pin table, assembly step 3, new calibration step 7, power-loop label ≈ 1.1A.

## Trade-offs

- ~2.7h per 3000mAh cell (vs ~8h) and closer to real-use load (~0.35C), but lower reported capacity than a slow discharge.
- Current not constant (0.9–1.26A); mAh integration handles it because I is computed every second.

## Implementation Results

- `R_LOAD` 10.0 → 3.33 in `src/battery_tester.cpp`; `pio run -e nanit_pro` builds clean (RAM 20.0%, Flash 11.8%).
- Schematic rev 07 rendered in headless Chrome; labels do not overlap.
- 2026-09-28: R1 measured 3.3Ω with the probes shorted at 0.1Ω, so R1 ≈ 3.2Ω (the meter resolves only 0.1Ω, ±3%). Effective loop R1 + Rds(on) ≈0.06 + wiring ≈ 3.3Ω, so `R_LOAD = 3.33` is kept until the V/I ammeter calibration.
- 2026-09-28 calibration (after the 013 wiring check and the 014 averaging): U_R1 = 3.66V, so I = 3.66 / 3.2 = 1.144A; TFT showed 1116 mA, implying V_A0 ≈ 1.116 × 3.33 ≈ 3.715V. Effective R = 3.715 / 1.144 ≈ **3.25Ω** (R1 3.2 + D–S 0.04V/1.14A ≈ 0.035 + wiring ≈ 0.015). `R_LOAD` 3.33 → 3.25; it is limited by the meter's 0.1Ω resolution on R1 (±3%).
