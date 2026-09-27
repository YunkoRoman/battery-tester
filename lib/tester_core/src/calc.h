#pragma once
#include <stdint.h>

namespace tc {

// SET pin PWM (0..255 of 5 V) → 39k / 10k / 10 µF → LM324 "+":
// V = duty / 255 * 5 * 10 / 49, and the CC loop makes I = V / 1 Ω.
const float kSetDividerRatio = 10.0f / 49.0f;

uint8_t currentToDuty(uint16_t milliamps);
uint16_t internalResistanceMilliOhm(float vOpen, float vLoad, float amps);
uint8_t liPoPercent(float volts);

}  // namespace tc
