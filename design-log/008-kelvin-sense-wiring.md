# 008 — Separate sense wiring (Kelvin) for the breadboard build

## Background

2026-09-24: circuit assembled on a breadboard, firmware (logs 002–007) flashed. On start, the multimeter showed the cell sagging from 4.0V to 3.66V under the 10Ω load, but the TFT showed ~2V and the firmware stopped the discharge within seconds — 5 samples ≤ 3.0V triggered the 005 cutoff, as designed for that input.

## Problem

A0 reads ~1.66V less than the cell's real loaded voltage, so every test ends right after start.

## Questions and Answers

- **Q1 — Is it the firmware?** No evidence of that. Given ~2V on A0, cutoff behaviour is correct. A wrong `V_REF` would make readings *higher* than real (actual AVcc ≤ 5V), not lower.
- **Q2 — Leading hypothesis?** Voltage drop in the shared load-current path. A0 measures P10_2 relative to Nanit GND; if those wires tap breadboard rails that also carry the ~0.35A load current, A0 sees V_cell − I·R_contacts. 1.66V / ~0.35A ≈ 4.5Ω — plausible for worn breadboard contacts plus 10W-resistor leads. The 0.34V sag at the cell itself also points at high loop resistance.
- **Q3 — Alternative hypothesis?** P10_2 wired to the MOSFET drain (R1/Q1 node) instead of Battery+, so A0 would read the drop across a partly-on MOSFET. Not yet ruled out.
- **Q4 — How to confirm?** Measure under load (~8s window after RESET: 3s start confirm + 5s to cutoff): cell terminals; P10_2↔Nanit GND; cell−↔Nanit GND; cell+↔P10_2; across R1. Not yet done — user went straight to the wiring fix.

## Design

Kelvin-style wiring (second diagram in `scratch_schematic.html`, rev 05):

- Power loop only: cell+ → R1 → Q1 drain, Q1 source → cell−. Short thick wires, soldered, no breadboard rails.
- Sense pair: separate thin wires from the holder terminals straight to P10_2 and Nanit GND. Only R4's ~0.4 mA flows in them, so A0 sees the true cell voltage.
- Signal side (R2, R3, R4, wires to Nanit) may stay on the breadboard.

## Trade-offs

- Soldering the power loop costs time, but no breadboard is reliable at 0.4A with 10W resistor leads.
- Remaining error: current is still computed as V_cell / 10Ω. Any leftover resistance in the power loop lowers the real current, so mAh reads high (~10% per extra 1Ω). Check the voltage across R1 vs cell voltage under load after rewiring.

## Implementation Results

- Breadboard wiring diagram and warning added to `scratch_schematic.html` (rev 05).
- 2026-09-25: power loop soldered, sense wires run separately from the holder to P10_2/Nanit GND, R3/R4 GND ends joined on the sense-GND wire. User confirms the circuit works — discharge no longer cuts off right after start. Separate measurements (A0 vs cell) weren't recorded, so the shared-path hypothesis (Q2) is confirmed by the fix working, not by a direct reading.
