#pragma once
#include <stdint.h>

#include "measure.h"

namespace tc {

const float kPowerResumeVolts = 4.7f;  // pause below kBusOkVolts (4.5 V), resume only above this
const uint8_t kPowerResumeTicks = 3;   // consecutive good 1 s samples before resuming

// +5V bus state with hysteresis, so a bus sagging around 4.5 V under charger load
// doesn't toggle every channel (and every relay) once a second.
class PowerMonitor {
  public:
    bool update(float busVolts);  // call once per tick; returns powerOk
    bool ok() const { return ok_; }

  private:
    bool ok_ = false;
    uint8_t good_ = 0;
};

}  // namespace tc
