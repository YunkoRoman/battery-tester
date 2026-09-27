#pragma once
#include <stdint.h>

#include "channel.h"
#include "ui_model.h"

namespace tc {

const uint16_t kServiceLoadMa = 200;
const float kServiceMinVolts = 2.8f;  // service LOAD never drains a cell below this

// Outputs for one channel in service mode. The state machine doesn't run there, so
// this is the only guard: an unsafe request is switched off and `o` reset to Off
// so the screen shows what the pins actually do. A missing NTC is allowed —
// stages 4–5 are checked before the NTC is fitted.
Outputs serviceOutputs(ServiceOut& o, const Measurement& m);

}  // namespace tc
