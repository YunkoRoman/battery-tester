#include "service.h"

#include "calc.h"

namespace tc {

Outputs serviceOutputs(ServiceOut& o, const Measurement& m) {
    Outputs out = {0, false};
    bool tempKnown = m.tempC != kTempInvalid;
    if (o == ServiceOut::Load) {
        if (m.cellV < kServiceMinVolts || (tempKnown && m.tempC > kHotAnyC)) {
            o = ServiceOut::Off;
            return out;
        }
        out.duty = currentToDuty(kServiceLoadMa);
    } else if (o == ServiceOut::Relay) {
        if (tempKnown && m.tempC > kHotChargeC) {
            o = ServiceOut::Off;
            return out;
        }
        out.relay = true;
    }
    return out;
}

}  // namespace tc
