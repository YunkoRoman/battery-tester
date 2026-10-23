# 007 — R4 = 10kΩ; single schematic source

## Background

006 specified R4 = 100kΩ (A0 pull-down). No 100kΩ resistor on hand; 10kΩ is (same part as R3). Separately, two schematics existed: `scratch_schematic.html` and the published Artifact built from `docs/schematic.html` (004), which had fallen behind (no R4, no new states).

## Problem

1. Use a resistor that's actually available for R4 without hurting capacity accuracy.
2. Keep exactly one schematic.

## Questions and Answers

- **Q1 — Is 10kΩ OK as a pull-down?** Yes. Draws V/10k ≈ 0.42 mA at 4.2V → ~3.4 mAh over an 8h test (~0.1% of 3000 mAh). 10kΩ is also the ATmega2560 ADC's recommended max source impedance, so reading accuracy is fine.
- **Q2 — Ignore or account for the extra drain?** Account. It's real charge leaving the cell, so current = V/R_LOAD + V/R_SENSE_PULLDOWN. Exact, one extra division per second.
- **Q3 — Which schematic survives?** `scratch_schematic.html` (local file, user's preferred page). Artifact `7NfC1sL7…` deleted, `docs/schematic.html` removed.

## Design

- `R_SENSE_PULLDOWN = 10000.0` in `battery_tester.cpp`; integration uses `voltage / R_LOAD + voltage / R_SENSE_PULLDOWN`.
- `scratch_schematic.html` rev 04: R4 labelled 10kΩ; BOM, assembly step 5, firmware formula updated.
- Logs 001/002/004 repointed from the deleted Artifact to `scratch_schematic.html`.

## Trade-offs

- 10kΩ drains 10× more than 100kΩ would, but it's measured and added to capacity, so no accuracy loss — only ~3 mAh taken from the cell that the load would otherwise have taken.
- The pull-down keeps drawing ~0.3 mA from a cell left in the holder after DONE. Don't leave cells in for days after a test.

## Implementation Results

- `pio run -e nanit_pro` builds clean (RAM 20.0%, Flash 11.8%).
- Artifact deleted; `docs/` removed.
- Not yet verified on hardware.
