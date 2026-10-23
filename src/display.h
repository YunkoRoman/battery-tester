#pragma once

#include <Arduino.h>

// Модуль виводу стану тестера на вбудований 1.8" TFT Nanit Pro (160x128, rotation 1).
// Вимагає, щоб Nanit_Base_Start() вже був викликаний (він ініціалізує глобальний `tft`).

enum class TesterState : uint8_t {
    NoBattery,    // напруга < NO_BATTERY_VOLTAGE — акумулятор не вставлено
    Idle,         // акумулятор є, але тест ще не стартував (≤ cutoff або йде підтвердження старту)
    Discharging,  // MOSFET відкритий, йде інтегрування ємності
    Done,         // розряд завершено по cutoff
    Removed       // акумулятор вийняли під час розряду — тест перервано
};

// Малює статичну частину екрана (заголовок, підписи). Викликати один раз у setup().
void displayInit();

// Оновлює значення на екрані. Перемальовує лише поля зі значеннями — без мерехтіння.
// nanitPercent — залишок власного Li-Po плати Nanit (0–100 %), нижній рядок екрана.
void displayUpdate(TesterState state, float voltage, float currentMa,
                   float capacityMah, unsigned long elapsedSec, uint8_t nanitPercent);
