# 002 — Firmware architecture

## Background

Design locked in 001 (`docs/plan.html`, `docs/build.html`). Firmware not started; plan in `docs/superpowers/plans/2026-09-27-firmware.md`.

## Problem

Structure the firmware so the decision logic (cycles, bad-cell rules, UI navigation, settings) can be tested without hardware, keep the AVR side thin, and give the user a way to check every soldering stage from the screen.

## Questions and Answers

- **Q1 — How to test logic without the board?** All logic in `lib/tester_core` (C++11, no Arduino headers), tested with PlatformIO `native` + Unity. `src/` only touches pins, TFT, EEPROM, timing.
- **Q2 — Timing model?** `tc::Channel::tick()` exactly once per second; timers (rest 10 min, charge timeout 4 h, SAG 5 min) and mAh integration count ticks. Buttons polled every loop.
- **Q3 — NO POWER detection?** 001 said "current set but shunt reads 0". That can't see power loss during charging (no shunt in the charge path). Changed to a 10k/10k divider from the +5V bus to **A7 · P1_1**: bus < 4.5 V ⇒ all running channels pause (outputs off, timers and integration frozen; Ri test restarts from scratch on resume).
- **Q4 — Why not A6?** NanitLib's `Nanit` constructor sets INPUT_PULLUP on P1_2 (A6) and enters a cable-test mode at boot if P1_2 and P12_2 are both LOW. A6 stays unused.
- **Q5 — Stage checks?** Service mode: hold OK at power-on. Rows per channel show V / I / T / STDBY and toggle OFF → RELAY → LOAD 0.20 A; last row = bus voltage + fan toggle. Hold OK exits with all outputs off.
- **Q6 — Keys?** ◀ ▶ move / change value, OK short = open / edit / confirm, OK hold ≥ 0.8 s = back to overview.
- **Q7 — Settings persistence?** EEPROM blob: magic 0xB7, version 1, 4 × {current, cutoff, mode}, crc8; blank / corrupt / out-of-range ⇒ defaults.
- **Q8 — Extra faults beyond 001?** `NTC?` (sensor open/short while running) and `NOI` (current set but not flowing with power present — wiring / MOSFET fault).

- **Q9 — Final review fixes?** (1) START refused when the cell is ≥ 40 °C (HOT) or its NTC is missing (NTC?) — the plan.html FULL-cycle precheck. (2) Service mode has its own guard (`tc::serviceOutputs`): LOAD drops below 2.8 V or above 55 °C, RELAY drops above 45 °C; a missing NTC is allowed so stages 4–5 can be checked before stage 6. (3) NO POWER with hysteresis (`tc::PowerMonitor`): pause below 4.5 V at once, resume only after 3 consecutive samples ≥ 4.7 V — a bus sagging under charger load no longer toggles relays every second. HOT > 55 °C applies to every running non-charge stage (rest and Ri test too), stricter than plan.html.

## Design

- `lib/tester_core`: `measure` (ADC → V / A / °C / bus), `calc` (PWM duty for a current, Ri, Nanit Li-Po %), `button`, `settings` (+ blob), `channel` (state machine), `ui_model` (screens + service), `view_text` (26-char display lines, integer formatting — AVR printf has no `%f`).
- `src`: `config.h` (pins), `board_io` (safe-off init, ADC averaging ×16, outputs with relay/load never overlapping), `display` (draws lines on the ST7735), `main.cpp` (1 s tick, keys, EEPROM, fan).
- Hardware addition: 2 × 10 kΩ bus divider (build stage 1). Docs updated: `build.html` stage 1 + "Перевірки в сервісному режимі", `plan.html` pins / NO POWER / keys.

## Trade-offs

- One extra divider (2 resistors, 1 analog pin) vs. a detection method that misses charging.
- 1 s tick is coarse for Ri (2 s under load) but matches the prototype and the ADC averaging budget.
- Service mode is part of the production firmware (no separate build env), so there's no compile_commands contention between envs.

## Implementation Results

- Status **drafted** — filled in after hardware bring-up (plan Task 10).
