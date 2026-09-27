# 001 — Four-channel tester: plan and build order

## Background

The 1-channel prototype (`../battery-capacity-tester`, logs 001–009 there) works: CC-less 10Ω discharge, Kelvin sense wiring, cutoff latch, no-battery detection, TFT display. The user wants a 4-channel version with charging, and the prototype is to stay unchanged — hence this separate project. Draft plan was first written as log 010 in the prototype project.

## Problem

4 independent channels, each able to: charge, discharge at a user-set current up to 1 A, measure capacity / internal resistance / temperature, reject bad cells; cells connect via holder or alligator clips; 3 buttons with overview + per-channel + settings screens; Nanit battery % on screen; everything stops when power is switched off.

## Questions and Answers

- **Q1 — Test flow?** Full auto cycle (FULL): check → charge → rest 10 min → Ri test → CC discharge to cutoff → rest → recharge. Also DISCHARGE-only and CHARGE-only.
- **Q2 — Power?** 5V 6A PSU (4 × TP4056 at 1 A + relays + fans ≈ 4.7 A), master switch + 5 A fuse → +5V/GND bus.
- **Q3 — Discharge current control?** Op-amp CC sink: LM324 (one quad chip for 4 channels), IRL540N on a shared insulated heatsink, 1Ω/5W shunt + 1Ω/5W ballast. Setpoint = PWM → 39k/10k/10µF → 0–1.02 V → 0–1.02 A.
- **Q4 — Charger switching?** 4-ch relay module, trigger jumper High + 10k pull-downs (dead Nanit ⇒ relays off). TP4056 without protection (B− = GND). End of charge via STDBY LED pad → INPUT_PULLUP.
- **Q5 — Temperature?** NTC 10k per cell; all dividers powered from one Nanit pin (NTC_PWR) so nothing is energised when Nanit is off.
- **Q6 — Nanit power / master switch?** Tested: Nanit does not run from USB with its switch OFF. Chosen: Nanit on USB from the bus, switch ON, Li-Po as backup. Master switch on the PSU line cuts all power paths; firmware detects "current set but shunt reads 0" → NO POWER, pauses channels, keeps data.
- **Q7 — Protect Nanit inputs from the cells?** Every cell-connected line goes through 10k in series (lesson from prototype log 009).
- **Q8 — Parts on hand / thresholds?** 4 × IRL540N and 4 holders on hand → buy only 1 MOSFET (fan). Defaults confirmed: Ri > 250 mΩ = BAD, cutoff 3.0 V, current 0.5 A.

## Design

- `docs/plan.html` — pin table (per channel V/I/T/SET/REL/DONE; buttons D14/D15/D43; FAN D10; NTC_PWR D31; Nanit Li-Po on A15), screen mockups (Latin-only font), cycle, bad-cell rules (DEAD, BAD Ri, SAG, HOT, CHG TIMEOUT, NO POWER, REMOVED), heat budget, BOM.
- `docs/build.html` — 9 stages in soldering order, each a small schematic + parts + steps + multimeter check: 1 power & buses, 2 buttons, 3 ch1 holder/clips/V-sense, 4 LM324 power & setpoint filter, 5 ch1 power loop/MOSFET/shunt/I-sense, 6 ch1 NTC, 7 relay module & ch1 TP4056, 8 fan, 9 channels 2–4 (pin table incl. LM324 pins per channel).
- Build order puts one full channel first so firmware can be brought up and verified on ch1 before replicating.

## Trade-offs

- Linear CC sink → heat (≤ 2.2 W per MOSFET at 1 A); ballast moves ~1 W to a cheap resistor; shared heatsink needs insulating kits because tabs are drains.
- 1Ω shunt on 5V-ref 10-bit ADC ≈ 4.9 mA/count — averaged in firmware.
- Relays click but need no soldering, and fail safe (open) when Nanit is off.
- Nanit Li-Po as backup means the board stays on after the master switch; handled by NO POWER state rather than a second switch.

## Implementation Results

- Project created: `platformio.ini`, `.clangd`, `lib/` (NanitLib, FastLed_Neopixel_Nanit) copied from the prototype; `src/` empty — firmware not started.
- `docs/plan.html` (moved from the prototype's `tester_4ch_plan.html`, updated for the power decision) and `docs/build.html` written and checked visually.
- 2026-09-27: open questions answered, BOM trimmed (holders and 4 MOSFETs on hand). Status **locked**; next step is the firmware implementation plan.
