#pragma once
#include <stdint.h>

namespace tc {

const float kAdcRefVolts = 5.0f;
const float kAdcMaxCounts = 1023.0f;
const float kShuntOhms = 1.0f;
const float kNtcSeriesOhms = 10000.0f;
const float kNtcR25Ohms = 10000.0f;
const float kNtcBeta = 3950.0f;
const float kBusDividerRatio = 2.0f;  // 10k/10k from the +5V bus to A7
const float kBusOkVolts = 4.5f;
const float kTempInvalid = -273.0f;   // open or shorted NTC

float adcToVolts(float counts);
float shuntAmps(float counts);
float busVolts(float counts);
bool busOk(float counts);
// NTC to GND, 10k series resistor from NTC_PWR (= ADC reference).
float ntcCelsius(float counts);

}  // namespace tc
