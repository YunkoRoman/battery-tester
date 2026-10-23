# 004 — Cutoff latch + schematic refresh

## Background

After the cutoff (3.0V) the gate goes LOW. With no load the cell's voltage recovers above 3.0V within seconds, and the old loop condition (`voltage > CUTOFF && !isDischarging`) restarted the discharge. Result: repeated start/stop cycles near cutoff, extra mAh added, display flipping DONE ↔ DISCHARGING. Separately, the schematic Artifact from 001 was gone and still showed the old 4× 10Ω bank.

## Problem

1. Once a test hits cutoff, it must stay finished until the user explicitly restarts it.
2. Publish a current schematic matching the single 10Ω load (002) and the display/state logic (003).

## Questions and Answers

- **Q1 — How does the user restart?** Board RESET (reboot). NanitLib has a touch `BUTTON` on connector J3, but it's an external module that may not be fitted; RESET needs no extra hardware and also clears `capacityMah`/timer naturally.
- **Q2 — What drives the DONE state now?** The `testFinished` flag, not `capacityMah > 0` — the state is explicit and can't flip back.
- **Q3 — Where does the schematic source live?** `docs/schematic.html` in the repo, so it can be edited and republished to the same URL. *(Superseded in 007: artifact and `docs/schematic.html` deleted, `scratch_schematic.html` is the only schematic.)*

## Design

Firmware (`battery_tester.cpp`):

- New `bool testFinished = false`.
- Start condition: `voltage > CUTOFF_VOLTAGE && !isDischarging && !testFinished`.
- On cutoff: gate LOW, `isDischarging = false`, `testFinished = true`.
- Display state: Discharging → Done (if `testFinished`) → Idle.

State machine: IDLE → DISCHARGING → DONE (terminal until RESET).

Schematic: https://claude.ai/artifact/7NfC1sL7Ajm1pCt1KuW89a *(deleted in 007)* — SVG circuit (18650, R1 10Ω/10W, Q1 IRL540N, R2 220Ω, R3 10kΩ pull-down, Nanit P10_2/P5_1/GND), BOM, pin map, firmware states.

## Trade-offs

- RESET vs touch button: no extra hardware, but no way to start a new test without losing the on-screen result. A button (J3 touch module or a spare Signal pin) can be added later if needed.
- Cutoff still triggers on a single 1s sample ≤ 3.0V. ADC noise or a load transient could end the test a bit early; averaging/debounce (e.g. N consecutive samples) is a possible follow-up.

## Implementation Results

- Latch in place; `pio run -e nanit_pro` builds clean (RAM 19.0%, Flash 11.7%).
- Schematic published: https://claude.ai/artifact/7NfC1sL7Ajm1pCt1KuW89a; logs 001/002 now link to it. Later deleted — see 007.
- Not yet verified on hardware.
