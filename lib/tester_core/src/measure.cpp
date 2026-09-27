#include "measure.h"

#include <math.h>

namespace tc {

float adcToVolts(float counts) { return counts * kAdcRefVolts / kAdcMaxCounts; }

float shuntAmps(float counts) { return adcToVolts(counts) / kShuntOhms; }

float busVolts(float counts) { return adcToVolts(counts) * kBusDividerRatio; }

bool busOk(float counts) { return busVolts(counts) >= kBusOkVolts; }

float ntcCelsius(float counts) {
    if (counts < 5.0f || counts > kAdcMaxCounts - 5.0f) return kTempInvalid;
    float ohms = kNtcSeriesOhms * counts / (kAdcMaxCounts - counts);
    float invKelvin = 1.0f / 298.15f + logf(ohms / kNtcR25Ohms) / kNtcBeta;
    return 1.0f / invKelvin - 273.15f;
}

}  // namespace tc
