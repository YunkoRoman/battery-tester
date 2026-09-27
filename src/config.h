#pragma once
#include <Arduino.h>

#include "settings.h"

// Nanit Pro v3.1 connector → Arduino pin. Source of truth: docs/plan.html "Піни Nanit".
struct ChannelPins {
    uint8_t v;     // cell voltage (through R5 10k)
    uint8_t i;     // shunt top (through 10k)
    uint8_t t;     // NTC divider
    uint8_t set;   // PWM → 39k/10k/10µF → LM324 "+"
    uint8_t rel;   // relay module IN (trigger High)
    uint8_t done;  // TP4056 STDBY LED, INPUT_PULLUP, LOW = charged
};

static const ChannelPins kPins[tc::kChannels] = {
    {A0, A2, A3, 6, 24, 28},     // ch1: P10_2 P10_1 P7_1 P10_4 P10_3 P7_3
    {A1, A12, A4, 2, 33, 22},    // ch2: P5_2 P5_3 P7_2 P5_1 P5_4 P6_4
    {A13, A14, A11, 9, 23, 25},  // ch3: P6_3 P6_2 P4_1 P7_4 P6_1 P3_2
    {A9, A10, A8, 5, 42, 30},    // ch4: P2_2 P2_1 P2_4 P8_3 P2_3 P3_3
};

static const uint8_t kButtonPins[3] = {14, 15, 43};  // ◀ P11_1, OK P11_2, ▶ P11_3
static const uint8_t kFanPin = 10;                   // P9_4
static const uint8_t kNtcPowerPin = 31;              // P3_4
static const uint8_t kBusSensePin = A7;              // P1_1, 10k/10k from the +5V bus. Not A6: NanitLib pulls it up.
static const uint8_t kNanitBatteryPin = A15;         // NanitLib BATTERY_PIN (69)
static const uint8_t kAdcSamples = 16;
