# 003 — TFT display module

## Background

Tester reported state only over Serial (9600 baud). Nanit Pro has a built-in 1.8" ST7735 TFT (160x128 after NanitLib's `setRotation(1)`), exposed as global `tft` after `Nanit_Base_Start()` — already proven by `nanit-hello-world`.

## Problem

Show live test data on the board's screen so the tester works standalone, without a PC on Serial. Display code must live in its own module, not in `battery_tester.cpp`.

## Questions and Answers

- **Q1 — Where does the module live?** `src/display.h` + `src/display.cpp`. PlatformIO compiles every `.cpp` in `src/`, so no `platformio.ini` change. Not a `lib/` library — it's project-specific and depends on NanitLib's global `tft`.
- **Q2 — What is shown?** Status (IDLE / DISCHARGING / DONE), voltage (V), current (mA), capacity (mAh), discharge duration (hh:mm:ss).
- **Q3 — How to avoid flicker?** Static labels drawn once in `displayInit()`. Values redrawn in place with `setTextColor(fg, bg)` and padded to fixed width, so old digits are overwritten — no `fillScreen`/`fillRect` per update.
- **Q4 — Who decides state?** Main sketch. Module is a pure view: takes `TesterState` + numbers, holds no tester logic.

## Design

API (`display.h`):

```cpp
enum class TesterState : uint8_t { Idle, Discharging, Done };
void displayInit();
void displayUpdate(TesterState state, float voltage, float currentMa,
                   float capacityMah, unsigned long elapsedSec);
```

Layout (160x128, value text size 2 = 12x16 px/char):

| y | Content |
|---|---|
| 4 | Status, colored (yellow DISCHARGING, green DONE, white IDLE) |
| 24 | `V` label + voltage, 2 decimals |
| 48 | `mA` label + current |
| 72 | `mAh` label + capacity |
| 100 | `Time` label + hh:mm:ss |

Main sketch changes: `displayInit()` in `setup()` after `Nanit_Base_Start()`; new `dischargeSeconds` counter incremented each 1s tick while discharging; `displayUpdate()` called every tick. State: `Discharging` if gate on, else `Done` if `capacityMah > 0`, else `Idle`.

## Trade-offs

- Free functions + anonymous namespace vs a `Display` class: only one screen and one global `tft`, so a class adds nothing now. Revisit at the 4-channel scale-out (per-channel rows).
- Full redraw of all fields every second (~few ms over SPI) — simpler than tracking changed values; negligible at 1 Hz.
- `dischargeSeconds` counts loop ticks, not `millis()` difference — same 1s resolution the mAh integration already assumes.

## Implementation Results

- `src/display.h`, `src/display.cpp` added; `battery_tester.cpp` wired up. Serial output unchanged.
- `pio run -e nanit_pro` builds clean (no warnings from project files). RAM 19.0%, Flash 11.7%.
- 2026-09-24: flashed; screen shows status and live voltage on the real board.
