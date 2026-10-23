#include <Arduino.h>
#include <NanitLib.h>

#include "display.h"

// Розряд підключено до Signal-пінів адаптерних роз'ємів плати Nanit Pro:
// GATE_PIN  -> роз'єм P5 (Signal 1) = D2 (PWM/INT0)
// VOLTAGE_PIN -> роз'єм P10 (Signal 2) = A0
const byte GATE_PIN = P5_1;      // Дигітальний пін керування MOSFET
const byte VOLTAGE_PIN = P10_2;  // Аналоговий пін вимірювання напруги

const float V_REF = 5.02;     // Напруга живлення АЦП — виміряно тестером на піні 5V Nanit (2026-09-28)
const float R_LOAD = 3.25;    // 3x 10Ω/10W паралельно (R1 ≈ 3.2Ω) + MOSFET + проводи; відкалібровано V_A0 / (U_R1 / 3.2Ω), 2026-09-28
const float CUTOFF_VOLTAGE = 3.0; // Порогова напруга зупинки розряду (В)
const byte CUTOFF_CONFIRM_SAMPLES = 5; // Скільки вимірювань поспіль ≤ cutoff потрібно для зупинки (фільтр шуму АЦП)
const byte START_CONFIRM_SAMPLES = 3;  // Скільки вимірювань поспіль > cutoff потрібно для старту розряду
const float NO_BATTERY_VOLTAGE = 0.5;  // Нижче — акумулятора немає (A0 підтягнутий до GND через R4)
const float R_SENSE_PULLDOWN = 10000.0; // R4: підтяжка A0 до GND, 10kΩ — теж розряджає акумулятор

unsigned long previousMillis = 0;
const long interval = 1000;   // Інтервал вимірювання (1 секунда)

float capacityMah = 0.0;      // Накопичена ємність в мА·год
bool isDischarging = false;
byte lowVoltageCount = 0;     // Лічильник вимірювань поспіль ≤ cutoff
byte highVoltageCount = 0;    // Лічильник вимірювань поспіль > cutoff (до старту)
bool testFinished = false;    // Фіксується після cutoff — повторний старт лише після RESET плати
bool testAborted = false;     // Фіксується, якщо акумулятор вийняли під час розряду
unsigned long dischargeSeconds = 0; // Тривалість розряду (с)

// Усереднення: 64 заміри рівномірно за ~20 мс (один період мережі 50 Гц) —
// гасить шум АЦП і наводки на провід до A0
const byte ADC_AVG_SAMPLES = 64;
const unsigned int ADC_AVG_GAP_US = 200; // + ~112 мкс на analogRead ≈ 312 мкс × 64 ≈ 20 мс

float readBatteryVoltage() {
    // Перший замір після перемикання каналу АЦП (A15 -> A0) неточний — відкидаємо
    analogRead(VOLTAGE_PIN);
    unsigned long sum = 0;
    for (byte i = 0; i < ADC_AVG_SAMPLES; i++) {
        sum += analogRead(VOLTAGE_PIN);
        delayMicroseconds(ADC_AVG_GAP_US);
    }
    float rawValue = (float)sum / ADC_AVG_SAMPLES;
    return (rawValue * V_REF) / 1023.0; // Для 10-бітного АЦП ATmega2560
}

// Крива розряду Li-Po Nanit: та сама таблиця, що в NanitLib getBatteryPower(),
// але без його бага (сегмент 3.68–3.8V там інтерполюється від 3.88V) і без
// провалів у 0% на точних межах сегментів (там строгі нерівності)
const float NANIT_CURVE_V[] = {4.24, 4.08, 4.06, 4.02, 3.98, 3.88, 3.80, 3.68, 3.54, 3.32, 3.00, 2.50};
const float NANIT_CURVE_P[] = {100.0, 85.0, 81.7, 76.7, 73.4, 58.4, 22.0, 8.7, 5.4, 2.1, 0.5, 0.0};
const byte NANIT_CURVE_POINTS = sizeof(NANIT_CURVE_V) / sizeof(NANIT_CURVE_V[0]);

// Залишок власного Li-Po плати Nanit (BATTERY_PIN = A15), 0–100 %
uint8_t readNanitPercent() {
    float v = GET_NANIT.getBatteryVoltage();
    if (v >= NANIT_CURVE_V[0]) return 100;
    for (byte i = 1; i < NANIT_CURVE_POINTS; i++) {
        if (v >= NANIT_CURVE_V[i]) {
            float t = (v - NANIT_CURVE_V[i]) / (NANIT_CURVE_V[i - 1] - NANIT_CURVE_V[i]);
            return (uint8_t)(NANIT_CURVE_P[i] + t * (NANIT_CURVE_P[i - 1] - NANIT_CURVE_P[i]) + 0.5);
        }
    }
    return 0;
}

void setup() {
    Nanit_Base_Start();
    Serial.begin(9600);
    pinMode(GATE_PIN, OUTPUT);
    digitalWrite(GATE_PIN, LOW); // Захисне вимкнення при старті

    displayInit();

    delay(1000);
    Serial.println("--- Battery Capacity Tester Initialized ---");
}

void loop() {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        float voltage = readBatteryVoltage();
        uint8_t nanitPercent = readNanitPercent();

        bool batteryPresent = voltage >= NO_BATTERY_VOLTAGE;

        // Логіка керування розрядом
        if (!isDischarging && !testFinished && !testAborted) {
            // Старт лише після кількох стабільних вимірювань > cutoff
            highVoltageCount = (voltage > CUTOFF_VOLTAGE) ? highVoltageCount + 1 : 0;
            if (highVoltageCount >= START_CONFIRM_SAMPLES) {
                digitalWrite(GATE_PIN, HIGH);
                isDischarging = true;
                lowVoltageCount = 0;
                Serial.println("Start discharging...");
            }
        } else if (isDischarging && !batteryPresent) {
            // Під навантаженням справжній акумулятор не падає нижче 0.5V —
            // це означає, що його вийняли (A0 тягнуть до GND R1 + MOSFET і R4)
            digitalWrite(GATE_PIN, LOW);
            isDischarging = false;
            testAborted = true;
            Serial.println("Battery REMOVED - test aborted!");
        } else if (isDischarging) {
            lowVoltageCount = (voltage <= CUTOFF_VOLTAGE) ? lowVoltageCount + 1 : 0;
            if (lowVoltageCount >= CUTOFF_CONFIRM_SAMPLES) {
                digitalWrite(GATE_PIN, LOW);
                isDischarging = false;
                // Без навантаження напруга відновлюється вище cutoff — без фіксації
                // розряд перезапустився б і завищив ємність
                testFinished = true;
                Serial.println("Discharge FINISHED!");
            }
        }

        // Обчислення ємності під час розряду
        float currentMa = 0.0;
        if (isDischarging) {
            // Струм з акумулятора: через навантаження R1 + через підтяжку R4 (~0.4 мА)
            float currentAmps = voltage / R_LOAD + voltage / R_SENSE_PULLDOWN;
            currentMa = currentAmps * 1000.0;
            capacityMah += currentMa / 3600.0;    // Додаємо мА·с за 1 секунду
            dischargeSeconds++;

            Serial.print("Voltage: "); Serial.print(voltage, 2); Serial.print("V | ");
            Serial.print("Current: "); Serial.print(currentMa, 1); Serial.print("mA | ");
            Serial.print("Capacity: "); Serial.print(capacityMah, 2); Serial.print("mAh | ");
            Serial.print("Nanit: "); Serial.print(nanitPercent); Serial.println("%");
        } else {
            Serial.print(batteryPresent ? "Battery Idle / Done. Voltage: " : "No battery. Voltage: ");
            Serial.print(voltage, 2);
            Serial.print("V | Nanit: ");
            Serial.print(nanitPercent);
            Serial.println("%");
        }

        TesterState state = isDischarging     ? TesterState::Discharging
                            : testAborted     ? TesterState::Removed
                            : testFinished    ? TesterState::Done
                            : !batteryPresent ? TesterState::NoBattery
                                              : TesterState::Idle;
        displayUpdate(state, voltage, currentMa, capacityMah, dischargeSeconds, nanitPercent);
    }
}
