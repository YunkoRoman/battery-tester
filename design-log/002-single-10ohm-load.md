# 002 — Load resistor: back to single 10Ω/10W

## Background

Log 001 (Q7) finalized the load at 4× 10Ω/10W in parallel (2.5Ω, 40W, I_max ≈ 1.68A). `R_LOAD = 2.5` in `battery_tester.cpp`.

## Problem

Decision reversed: use a single 10Ω/10W resistor as the discharge load instead of the 4-resistor parallel bank.

## Questions and Answers

- **Q1 — Is one 10Ω/10W within rating?** Yes. Worst case at 4.2V: I = 0.42A, P = V²/R ≈ 1.76W — ~18% of the 10W rating. Resistor still gets warm; keep it off the board/plastic.
- **Q2 — MOSFET dissipation?** I²×Rds(on) ≈ 0.42² × 0.06 ≈ 0.01W — negligible, no heatsink.
- **Q3 — Test duration impact?** Average current ≈ 0.37A (≈3.7V mean). A 3000mAh cell takes ~8h vs ~2h with the 2.5Ω bank. Lower C-rate (~0.12C) reads closer to nominal capacity than the ~0.5C bank would.

## Design

```
Battery(+) ──┬── [10Ω/10W] ── MOSFET Drain
             └── Nanit P10_2 (A0)   MOSFET Source ── GND (common)
                                     MOSFET Gate ── 220Ω ── Nanit P5_1 (D2)
                                     MOSFET Gate ── 10kΩ ── GND (pull-down)
```

Firmware: `R_LOAD = 10.0` (current = V / R_LOAD). No other logic change.

## Trade-offs

- **Single 10Ω vs 4× parallel**: simpler wiring, cooler, 3 resistors freed for the planned 4-channel scale-out (one 10Ω per channel). Cost: ~4× longer test per cell.
- Current is still computed as V/R using the nominal 10Ω; resistor tolerance (typ. ±5%) directly scales mAh error. Measuring the actual resistor with a multimeter and putting that value into `R_LOAD` improves accuracy.

## Implementation Results

- `R_LOAD` changed 2.5 → 10.0 in `src/battery_tester.cpp`; `pio run -e nanit_pro` builds clean.
- Schematic updated to single 10Ω in 004/005; current source `scratch_schematic.html` (see 007).
- Physical circuit assembled on a breadboard (2026-09-24); multimeter showed the cell sagging 4.0V → 3.66V under the 10Ω load (see 008).
