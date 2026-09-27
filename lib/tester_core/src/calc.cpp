#include "calc.h"

namespace tc {

uint8_t currentToDuty(uint16_t milliamps) {
    float duty = milliamps / 1000.0f * 255.0f / (5.0f * kSetDividerRatio);
    if (duty >= 255.0f) return 255;
    return (uint8_t)(duty + 0.5f);
}

uint16_t internalResistanceMilliOhm(float vOpen, float vLoad, float amps) {
    if (amps < 0.05f || vOpen <= vLoad) return 0;
    float milliOhm = (vOpen - vLoad) / amps * 1000.0f;
    if (milliOhm > 65535.0f) return 65535;
    return (uint16_t)(milliOhm + 0.5f);
}

namespace {

struct LiPoPoint {
    float volts;
    float percent;
};

// Replaces NanitLib's getBatteryPower() table, whose 3.68–3.80 V segment uses 3.88.
const LiPoPoint kLiPo[] = {
    {4.20f, 100.0f}, {4.08f, 85.0f}, {4.02f, 77.0f}, {3.98f, 73.0f},
    {3.88f, 58.0f},  {3.80f, 22.0f}, {3.68f, 9.0f},  {3.54f, 5.0f},
    {3.32f, 2.0f},   {3.00f, 0.5f},  {2.50f, 0.0f},
};
const uint8_t kLiPoCount = sizeof(kLiPo) / sizeof(kLiPo[0]);

}  // namespace

uint8_t liPoPercent(float volts) {
    if (volts >= kLiPo[0].volts) return 100;
    for (uint8_t i = 1; i < kLiPoCount; i++) {
        if (volts >= kLiPo[i].volts) {
            const LiPoPoint& hi = kLiPo[i - 1];
            const LiPoPoint& lo = kLiPo[i];
            float pct = lo.percent + (volts - lo.volts) * (hi.percent - lo.percent) / (hi.volts - lo.volts);
            return (uint8_t)(pct + 0.5f);
        }
    }
    return 0;
}

}  // namespace tc
