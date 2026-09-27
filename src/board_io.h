#pragma once
#include <stdint.h>

#include "channel.h"

namespace io {

void begin();  // every output safe-off; call first in setup()
void apply(uint8_t ch, const tc::Outputs& o);
void measureAll(tc::Measurement* out, bool powerOk);
float busVolts();
float nanitBatteryVolts();
bool buttonDown(uint8_t idx);  // 0 = ◀, 1 = OK, 2 = ▶
void setFan(bool on);

}  // namespace io
