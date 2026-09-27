#include "channel.h"

#include "calc.h"

namespace tc {

namespace {
const Outputs kOff = {0, false};
}

Channel::Channel()
    : settings_(defaultSettings()),
      stage_(Stage::Empty),
      fault_(Fault::None),
      paused_(false),
      stageSeconds_(0),
      confirm_(0),
      noCurrent_(0),
      vOpen_(0.0f),
      lastV_(0.0f),
      lastI_(0.0f),
      lastT_(kTempInvalid),
      results_() {}

bool Channel::running() const {
    switch (stage_) {
        case Stage::Charge:
        case Stage::RestCharged:
        case Stage::RiTest:
        case Stage::Discharge:
        case Stage::RestDischarged:
        case Stage::Recharge:
            return true;
        default:
            return false;
    }
}

void Channel::configure(const ChannelSettings& s) {
    if (!running()) settings_ = s;
}

void Channel::enter(Stage s) {
    stage_ = s;
    stageSeconds_ = 0;
    confirm_ = 0;
    noCurrent_ = 0;
}

void Channel::fail(Fault f) {
    fault_ = f;
    enter(Stage::Fault);
}

bool Channel::start() {
    if (running() || lastV_ < kPresentVolts) return false;
    results_ = Results();
    fault_ = Fault::None;
    paused_ = false;
    if (lastV_ < kDeadVolts) {
        fail(Fault::Dead);
        return true;
    }
    enter(settings_.mode == Mode::Discharge ? Stage::RiTest : Stage::Charge);
    return true;
}

void Channel::stop() {
    fault_ = Fault::None;
    paused_ = false;
    enter(lastV_ >= kPresentVolts ? Stage::Idle : Stage::Empty);
}

Outputs Channel::tick(const Measurement& m) {
    lastV_ = m.cellV;
    lastI_ = m.currentA;
    lastT_ = m.tempC;

    if (!running()) {
        paused_ = false;
        if (stage_ == Stage::Empty && m.cellV >= kPresentVolts) enter(Stage::Idle);
        else if (stage_ == Stage::Idle && m.cellV < kPresentVolts) enter(Stage::Empty);
        return kOff;
    }

    if (!m.powerOk) {
        paused_ = true;
        if (stage_ == Stage::RiTest) enter(Stage::RiTest);  // measure again from scratch
        return kOff;
    }
    paused_ = false;

    if (m.cellV < kPresentVolts) {
        fail(Fault::Removed);
        return kOff;
    }
    if (m.tempC == kTempInvalid) {
        fail(Fault::SensorError);
        return kOff;
    }
    if (m.tempC > results_.maxTempC) results_.maxTempC = m.tempC;
    bool charging = stage_ == Stage::Charge || stage_ == Stage::Recharge;
    if (m.tempC > kHotAnyC || (charging && m.tempC > kHotChargeC)) {
        fail(Fault::Hot);
        return kOff;
    }

    stageSeconds_++;
    switch (stage_) {
        case Stage::Charge:
            return chargeTick(m, settings_.mode == Mode::Full ? Stage::RestCharged : Stage::Done);
        case Stage::Recharge:
            return chargeTick(m, Stage::Done);
        case Stage::RestCharged:
            if (stageSeconds_ >= kRestSeconds) enter(Stage::RiTest);
            return kOff;
        case Stage::RestDischarged:
            if (stageSeconds_ >= kRestSeconds) enter(Stage::Recharge);
            return kOff;
        case Stage::RiTest:
            return riTick(m);
        case Stage::Discharge:
            return dischargeTick(m);
        default:
            return kOff;
    }
}

Outputs Channel::chargeTick(const Measurement& m, Stage next) {
    if (stageSeconds_ >= kChargeTimeoutSeconds) {
        fail(Fault::ChargeTimeout);
        return kOff;
    }
    confirm_ = m.chargeDone ? (uint8_t)(confirm_ + 1) : 0;
    if (confirm_ >= kChargeDoneConfirm) {
        enter(next);
        return kOff;
    }
    Outputs o = {0, true};
    return o;
}

Outputs Channel::riTick(const Measurement& m) {
    Outputs load = {currentToDuty(settings_.currentMa), false};
    if (stageSeconds_ == 1) {
        vOpen_ = m.cellV;  // this sample was taken with the load off
        return load;
    }
    if (stageSeconds_ < 1u + kRiSettleSeconds) return load;
    if (m.currentA < kMinRiCurrentA) {
        fail(Fault::NoCurrent);
        return kOff;
    }
    results_.riMilliOhm = internalResistanceMilliOhm(vOpen_, m.cellV, m.currentA);
    if (results_.riMilliOhm > kBadRiMilliOhm) {
        fail(Fault::BadRi);
        return kOff;
    }
    if (m.cellV <= cutoffVolts()) {
        fail(Fault::Sag);
        return kOff;
    }
    enter(Stage::Discharge);
    return load;
}

Outputs Channel::dischargeTick(const Measurement& m) {
    Outputs load = {currentToDuty(settings_.currentMa), false};
    results_.mAh += m.currentA * 1000.0f / 3600.0f;
    results_.mWh += m.currentA * m.cellV * 1000.0f / 3600.0f;
    results_.dischargeSeconds++;

    float setAmps = settings_.currentMa / 1000.0f;
    noCurrent_ = (m.currentA < kNoCurrentFraction * setAmps) ? (uint8_t)(noCurrent_ + 1) : 0;
    if (noCurrent_ >= kNoCurrentConfirm) {
        fail(Fault::NoCurrent);
        return kOff;
    }

    confirm_ = (m.cellV <= cutoffVolts()) ? (uint8_t)(confirm_ + 1) : 0;
    if (confirm_ >= kCutoffConfirm) {
        if (results_.dischargeSeconds < kMinDischargeSeconds) {
            fail(Fault::Sag);
            return kOff;
        }
        enter(settings_.mode == Mode::Full ? Stage::RestDischarged : Stage::Done);
        return kOff;
    }
    return load;
}

}  // namespace tc
