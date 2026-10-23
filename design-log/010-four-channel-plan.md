# 010 — Four-channel tester with charging (plan, draft)

## Background

1-channel prototype works (008). User wants 4 channels, each with its own charging, alligator-clip option, discharge current adjustable up to 1A, 3 buttons, overview + per-channel screens, Nanit battery % on screen, bad-cell detection, 5V fans available.

## Problem

Design the circuit, pinout, UI and BOM for 4 independent channels, with everything shutting off when Nanit or the PSU is off.

## Questions and Answers

- **Q1 — Test flow?** Full auto cycle: charge → rest → Ri test → CC discharge → rest → recharge. Also DISCHARGE-only and CHARGE-only modes.
- **Q2 — Power?** 5V 6A PSU (4 × TP4056 at 1A + relays + fans ≈ 4.7A), master switch + 5A fuse.
- **Q3 — Discharge current control?** Op-amp constant-current sink per channel: LM324 (quad), IRL540N, 1Ω/5W shunt, 1Ω/5W ballast; setpoint from PWM via 39k/10k/10µF filter (0–1.02V → 0–1.02A).
- **Q4 — Charger switching?** 4-ch 5V relay module, trigger jumper on High + 10k pull-downs so a dead Nanit means relays off. TP4056 *without* protection (B− = GND). End-of-charge from the STDBY LED pad → INPUT_PULLUP.
- **Q5 — Temperature?** NTC 10k per cell, dividers powered from one Nanit pin (NTC_PWR) so nothing is energised when Nanit is off.
- **Open:** Nanit power (own Li-Po + switch vs USB from PSU with PSU switch as master); how many IRL540N / holders on hand; default thresholds.

## Design

Full draft: `tester_4ch_plan.html` — three per-channel diagrams (CC discharge, charge, measurement), pin table (4 × V/I/T/SET/REL/DONE + 3 buttons, fan, NTC_PWR; Nanit Li-Po on A15), UI mockups (overview, channel, settings; Latin-only font), FULL cycle, bad-cell rules (DEAD < 2.0V, Ri > 250 mΩ, SAG, HOT 45/55 °C, CHG TIMEOUT 4h, REMOVED), heat budget (≤ 2.2W per MOSFET, shared insulated heatsink + fan), shopping list.

## Trade-offs

- Linear CC sink → heat in MOSFETs; ballast resistor moves half of it to a cheap 5W part.
- 1Ω shunt on a 5V-ref 10-bit ADC gives ~4.9 mA/count; averaging covers it. Internal 2.56V ref would double resolution but complicates mixed readings.
- Relays click but need zero soldering vs P-MOSFET high-side switches.

## Implementation Results

- Status **drafted**: awaiting user review of `tester_4ch_plan.html` and answers to the open questions. After approval → written spec → implementation plan.
- 2026-09-27: moved to its own project `../battery-tester-4ch` (log 001 there); `tester_4ch_plan.html` now lives at `../battery-tester-4ch/docs/plan.html`. This project stays as the 1-channel prototype.
