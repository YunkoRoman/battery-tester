// Phase 1 firmware — build stage 1: power entry, +5V bus, bus divider on A7, Nanit on USB.
//
// Shows the +5V bus voltage, POWER OK / NO POWER (with the same hysteresis the full
// firmware uses) and the Nanit Li-Po. Every channel output pin is held safe-off:
// the circuits that use them are added in later phases.
#include <Arduino.h>
#include <NanitLib.h>

#include "calc.h"
#include "config.h"
#include "measure.h"
#include "power.h"

namespace {

const uint16_t kBg = ST7735_BLACK;
const uint16_t kGrey = 0x7BEF;

tc::PowerMonitor power;
uint32_t lastTickMs = 0;

float readAdc(uint8_t pin) {
    analogRead(pin);  // discard: sample-and-hold settles after the mux switch
    uint16_t sum = 0;
    for (uint8_t k = 0; k < kAdcSamples; k++) sum += analogRead(pin);
    return (float)sum / kAdcSamples;
}

void holdOutputsOff() {
    for (uint8_t c = 0; c < tc::kChannels; c++) {
        pinMode(kPins[c].set, OUTPUT);
        digitalWrite(kPins[c].set, LOW);
        pinMode(kPins[c].rel, OUTPUT);
        digitalWrite(kPins[c].rel, LOW);
    }
    pinMode(kNtcPowerPin, OUTPUT);
    digitalWrite(kNtcPowerPin, LOW);
    pinMode(kFanPin, OUTPUT);
    digitalWrite(kFanPin, LOW);
    pinMode(kBusSensePin, INPUT);
}

void drawStatic() {
    tft.setRotation(1);
    tft.setTextWrap(false);
    tft.fillScreen(kBg);
    tft.setTextSize(1);
    tft.setTextColor(ST7735_CYAN, kBg);
    tft.setCursor(2, 4);
    tft.print("PHASE 1: POWER + BUS");
    tft.setTextColor(kGrey, kBg);
    tft.setCursor(2, 24);
    tft.print("+5V BUS (A7 x2)");
    tft.setCursor(2, 112);
    tft.print("switch OFF -> NO POWER");
}

void draw(float busV, bool powerOk, float liPoV) {
    tft.setTextSize(2);
    tft.setTextColor(ST7735_WHITE, kBg);
    tft.setCursor(2, 38);
    tft.print(busV, 2);
    tft.print(" V   ");

    tft.setCursor(2, 64);
    if (powerOk) {
        tft.setTextColor(ST7735_GREEN, kBg);
        tft.print("POWER OK  ");
    } else {
        tft.setTextColor(ST7735_MAGENTA, kBg);
        tft.print("NO POWER  ");
    }

    tft.setTextSize(1);
    tft.setTextColor(ST7735_WHITE, kBg);
    tft.setCursor(2, 92);
    tft.print("NANIT ");
    tft.print(liPoV, 2);
    tft.print("V ");
    tft.print(tc::liPoPercent(liPoV));
    tft.print("%   ");
}

}  // namespace

void setup() {
    holdOutputsOff();    // before anything else
    Nanit_Base_Start();  // TFT init; touches only TFT pins
    Serial.begin(9600);
    drawStatic();
    lastTickMs = millis();
}

void loop() {
    const uint32_t now = millis();
    if (now - lastTickMs < 1000) return;
    lastTickMs += 1000;

    float busV = tc::busVolts(readAdc(kBusSensePin));
    bool powerOk = power.update(busV);
    float liPoV = tc::adcToVolts(readAdc(kNanitBatteryPin));
    draw(busV, powerOk, liPoV);

    Serial.print(busV, 2);
    Serial.print(',');
    Serial.print(powerOk ? 1 : 0);
    Serial.print(',');
    Serial.println(liPoV, 2);
}
