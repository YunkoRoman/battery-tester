# battery-tester-4ch

4-channel 18650 capacity tester on a Nanit Pro (ATmega2560): charge (TP4056), constant-current discharge 0.1–1.0 A (LM324 + IRL540N), capacity / internal resistance / temperature, bad-cell rejection.

- `docs/phase_1.html` — phase 1: schematic, assembly, flashing, expected screen, checks
- `docs/layout.html` — physical layout on a 400×280 mm board: 3D view, top-view template, side section, mounting, cover
- `docs/schematics.html` — overall schematic, per-stage schematics, shopping list
- `docs/build.html` — soldering stages with checks
- `docs/plan.html` — pins, screens, cycle logic
- `design-log/` — design decisions

## Build in phases

The tester is built one circuit at a time. Each phase has its own branch with firmware that exercises only what has been soldered so far: solder the stage → flash that branch → check → next phase.

| Branch | Build stage (`docs/build.html`) | What the firmware shows / does |
| --- | --- | --- |
| `phase_1` | 1 — power, +5V bus, bus divider → A7, USB to Nanit | Bus voltage, POWER OK / NO POWER, Nanit Li-Po. All channel outputs held off. |
| `phase_2` | 2 — buttons | Key presses (short / long) on screen |
| `phase_3` | 3 — ch1 holder, clips, voltage sense | Ch1 cell voltage, cell present / empty |
| `phase_4` | 4–5 — LM324, ch1 power loop, shunt | Ch1 constant-current load, set vs measured current |
| `phase_5` | 6 — ch1 NTC | Ch1 temperature |
| `phase_6` | 7 — relay module, ch1 TP4056 | Ch1 charge on/off, STDBY |
| `phase_7` | 8 — fan | Fan on/off |
| `phase_8` | — | Full firmware on ch1: FULL / DISCHARGE / CHARGE cycles, UI, bad-cell rules |
| `phase_9` | 9 — channels 2–4 | Full 4-channel firmware |

The complete firmware (all phases) is on branch `firmware`.

## Build / flash / test

```bash
git clone --recurse-submodules https://github.com/YunkoRoman/battery-tester.git
cd battery-tester && git checkout phase_1
pio run -e nanit_pro -t upload     # flash the Nanit (USB)
pio device monitor -b 9600         # serial log
pio test -e native                 # logic tests on the PC
```
