#include "board_io.h"

#include <Arduino.h>

#include "config.h"
#include "measure.h"

namespace io {

namespace {

float readAdc(uint8_t pin) {
    analogRead(pin);  // discard: sample-and-hold settles after the mux switch
    uint16_t sum = 0;
    for (uint8_t k = 0; k < kAdcSamples; k++) sum += analogRead(pin);
    return (float)sum / kAdcSamples;
}

}  // namespace

void begin() {
    for (uint8_t c = 0; c < tc::kChannels; c++) {
        pinMode(kPins[c].set, OUTPUT);
        analogWrite(kPins[c].set, 0);
        pinMode(kPins[c].rel, OUTPUT);
        digitalWrite(kPins[c].rel, LOW);
        pinMode(kPins[c].done, INPUT_PULLUP);
    }
    pinMode(kNtcPowerPin, OUTPUT);
    digitalWrite(kNtcPowerPin, LOW);
    pinMode(kFanPin, OUTPUT);
    digitalWrite(kFanPin, LOW);
    for (uint8_t b = 0; b < 3; b++) pinMode(kButtonPins[b], INPUT_PULLUP);
    pinMode(kBusSensePin, INPUT);
}

void apply(uint8_t ch, const tc::Outputs& o) {
    // Switch the old path off first so relay and load never overlap.
    if (o.relay) {
        analogWrite(kPins[ch].set, 0);
        digitalWrite(kPins[ch].rel, HIGH);
    } else {
        digitalWrite(kPins[ch].rel, LOW);
        analogWrite(kPins[ch].set, o.duty);
    }
}

void measureAll(tc::Measurement* out, bool powerOk) {
    digitalWrite(kNtcPowerPin, HIGH);
    delay(2);
    for (uint8_t c = 0; c < tc::kChannels; c++) {
        out[c].cellV = tc::adcToVolts(readAdc(kPins[c].v));
        out[c].currentA = tc::shuntAmps(readAdc(kPins[c].i));
        out[c].tempC = tc::ntcCelsius(readAdc(kPins[c].t));
        out[c].chargeDone = digitalRead(kPins[c].done) == LOW;
        out[c].powerOk = powerOk;
    }
    digitalWrite(kNtcPowerPin, LOW);
}

float busVolts() { return tc::busVolts(readAdc(kBusSensePin)); }

float nanitBatteryVolts() { return tc::adcToVolts(readAdc(kNanitBatteryPin)); }

bool buttonDown(uint8_t idx) { return digitalRead(kButtonPins[idx]) == LOW; }

void setFan(bool on) { digitalWrite(kFanPin, on ? HIGH : LOW); }

}  // namespace io
