# 009 — R5: series resistor on A0 (stop the tested cell powering the board)

## Background

A0 (P10_2) is wired straight to the cell's "+". The user noticed the board seems to stay powered from the cell under test when the Nanit power switch is off.

## Problem

With Nanit's supply off (0V), 3–4.2V on A0 forward-biases the pin's internal ESD diode to VCC and feeds the whole board from the tested cell. The MCU can then run brown-out-ish and leave the MOSFET gate high — discharge continues unsupervised, below cutoff.

## Questions and Answers

- **Q1 — Can firmware see the power switch?** No. NanitLib exposes no switch pin; the switch physically disconnects the Li-Po. (NanitLib does read the Li-Po on `BATTERY_PIN` 69 / A15 via `getBatteryVoltage()` / `getBatteryPower()`; the % table has a bug in the 3.68–3.8V segment — uses 3.88.)
- **Q2 — Fix?** R5 = 10kΩ in series between the R4 node (cell side) and A0. Injection current ≤ (4.2 − 0.5)/10k ≈ 0.37 mA — far below what the board needs to run, inside the ATmega's pin-injection comfort zone. With the MCU unpowered, R3 holds the gate LOW, discharge stops.
- **Q3 — Does R5 change readings?** No. R4 stays on the cell side, so no divider forms; the ADC input draws ~no current. 10kΩ source impedance is the ATmega ADC's recommended max.
- **Q4 — Symptom confirmed?** Not yet — asked the user what they saw with the switch off.

## Design

```
cell+ ──┬── R5 10kΩ ──► P10_2 (A0)
        R4 10kΩ
        │
       GND
```

`scratch_schematic.html` rev 06: R5 in both diagrams, BOM, pin map, assembly step 5, warning block about the power switch.

## Trade-offs

- No firmware change needed. Firmware still can't tell "switch off" from "USB powered"; if Nanit runs from USB, the switch doesn't stop it — addressed in the 4-channel plan (010).

## Implementation Results

- Schematic updated (rev 06). Not yet fitted on hardware.
