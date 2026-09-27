# 005 — Power entry: 14V monitor adapter + XL4016 buck

## Background

Log 001 planned a 5V 6A PSU feeding the +5V bus directly. The user has no such PSU; they have a 12.6V 5A 3S Li-ion charger and a 14V 35W monitor adapter.

## Problem

Feed the +5V bus (4 × TP4056 at 1 A, relay module, LM324, fans, Nanit over USB — ≈ 4.7 A worst case) from a higher-voltage adapter without risking the 5 V parts.

## Questions and Answers

- **Q1 — 3S charger or monitor adapter?** Monitor adapter. A CC/CV charger may gate or hunt its output until it sees a battery; a monitor adapter is a plain constant-voltage supply.
- **Q2 — Is 35 W enough?** 4.7 A × 5 V ≈ 24 W on the bus, ≈ 26 W from the adapter at ~90 % buck efficiency (≈ 1.9 A at 14 V) — ~75 % load. Enough for 4 channels, not for 8. Fallback if the adapter runs hot: firmware cap of 3 concurrent charges (≈ 20 W).
- **Q3 — Which buck?** XL4016 (8 A rated, 5–6 A realistic with its heatsink, 40 V input). LM2596 (≈ 3 A real) is too small.
- **Q4 — What if the buck fails shorted?** 14 V on the bus would kill TP4056 (8 V abs max) and the Nanit. SMBJ5.0A TVS across the bus clamps and fails short; the 3 A input fuse / adapter OCP then cuts power.

## Design

```
14V 35W → DC jack → switch → fuse 3A → XL4016 (set 5.05 V) → fuse 6A → +5V bus
                                                                ├─ SMBJ5.0A (cathode to +5V) → GND
                                                                └─ 10k/10k → A7 (unchanged)
```

- XL4016 output is set to 5.05 V with nothing connected, before the bus is wired; trimmer locked with a drop of glue.
- Bus sensing, NO POWER thresholds (4.5 / 4.7 V) and all firmware unchanged.
- No mains on the board any more: the adapter brick sits off-board; layout's 220 V warnings replaced by the XL4016 set-before-connect warning.

## Trade-offs

- Extra part and a second conversion stage (~2–3 W lost in the buck, needs airflow).
- Power budget is tighter than the 30 W 5V PSU (≈ 75 % vs ≈ 80 % of rating, but with a separate buck limit to watch).
- 8+ channels would need a larger adapter (≥ 60 W) and a 10 A buck.

## Implementation Results

- Docs updated: stage-1 schematic (phase_1.html, build.html, schematics.html), power rows and current budget (plan.html), BOM (plan.html, schematics.html), overall/charge schematic labels, layout.html (DC jack + XL4016 + two fuses replace the 5V PSU), CLAUDE.md power line.
- Firmware: no change.
