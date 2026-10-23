# 005 — Cutoff debounce (N consecutive samples)

## Background

Log 004 latched the test as finished on cutoff, but the cutoff itself fired on a single 1s sample ≤ 3.0V. One noisy ADC reading or a short load transient could end the test early — and since 004 the latch makes that early stop final until RESET.

## Problem

Stop discharge only when the cell is really at cutoff, not on one outlier sample.

## Questions and Answers

- **Q1 — Averaging or consecutive-sample count?** Consecutive count. Simple, no buffer, and a real cutoff crossing under load is monotonic — voltage stays low once it gets there.
- **Q2 — How many samples?** `CUTOFF_CONFIRM_SAMPLES = 5` (5s at the 1s poll). Extra charge drawn past cutoff ≈ 0.3A × 5s ≈ 0.4 mAh — negligible, and the cell only sees a few extra seconds under 3.0V.
- **Q3 — Keep integrating during confirmation?** Yes. The gate is still on and current still flows, so those seconds count toward mAh.

## Design

- `lowVoltageCount` increments on each sample ≤ `CUTOFF_VOLTAGE` while discharging, resets to 0 on any sample above it.
- Discharge stops (gate LOW, `testFinished = true`) once `lowVoltageCount >= CUTOFF_CONFIRM_SAMPLES`.
- Start condition and latch from 004 unchanged.

## Trade-offs

- Up to 5s of discharge below 3.0V vs false early stops. 3.0V has margin over typical 2.5–2.75V absolute minimum, so a few seconds under it is safe.
- A spike *above* cutoff resets the counter — noisy readings near the end can delay the stop a bit longer. Acceptable at 1 Hz; switch to a moving average if logs show chatter.

## Implementation Results

- `pio run -e nanit_pro` builds clean (RAM 19.1%, Flash 11.7%).
- `scratch_schematic.html` (rev 02) updated: single R1 10Ω/10W instead of the 4× bank, BOM/assembly steps fixed, new "Логіка прошивки" section with IDLE/DISCHARGING/DONE, 5-sample cutoff, RESET latch and TFT contents.
- Not yet verified on hardware.
