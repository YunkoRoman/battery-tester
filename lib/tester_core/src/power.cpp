#include "power.h"

namespace tc {

bool PowerMonitor::update(float busVolts) {
    if (busVolts < kBusOkVolts) {
        ok_ = false;
        good_ = 0;
        return ok_;
    }
    if (ok_) return ok_;
    good_ = busVolts >= kPowerResumeVolts ? (uint8_t)(good_ + 1) : 0;
    if (good_ >= kPowerResumeTicks) ok_ = true;
    return ok_;
}

}  // namespace tc
