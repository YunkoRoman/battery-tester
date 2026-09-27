#pragma once
#include <stdint.h>

#include "measure.h"
#include "settings.h"

namespace tc {

enum class Stage : uint8_t { Empty, Idle, Charge, RestCharged, RiTest, Discharge, RestDischarged, Recharge, Done, Fault };
enum class Fault : uint8_t { None, Dead, BadRi, Sag, Hot, ChargeTimeout, Removed, SensorError, NoCurrent };

struct Measurement {
    float cellV;
    float currentA;
    float tempC;      // kTempInvalid if the NTC is open/shorted
    bool chargeDone;  // TP4056 STDBY active
    bool powerOk;     // +5V bus present
};

struct Outputs {
    uint8_t duty;  // SET pin PWM, 0 = no discharge
    bool relay;    // charger power
};

struct Results {
    float mAh;
    float mWh;
    uint16_t riMilliOhm;
    float maxTempC;
    uint32_t dischargeSeconds;
};

const float kPresentVolts = 0.5f;
const float kDeadVolts = 2.0f;
const uint16_t kBadRiMilliOhm = 250;
const float kHotChargeC = 45.0f;
const float kHotAnyC = 55.0f;
const uint32_t kRestSeconds = 600;
const uint32_t kChargeTimeoutSeconds = 4UL * 3600UL;
const uint32_t kMinDischargeSeconds = 300;
const uint8_t kCutoffConfirm = 5;
const uint8_t kChargeDoneConfirm = 3;
const uint8_t kRiSettleSeconds = 2;
const uint8_t kNoCurrentConfirm = 5;
const float kMinRiCurrentA = 0.05f;
const float kNoCurrentFraction = 0.3f;

class Channel {
  public:
    Channel();

    void configure(const ChannelSettings& s);  // ignored while running
    const ChannelSettings& settings() const { return settings_; }

    bool start();  // false if no cell; resets results
    void stop();   // outputs off, back to Idle / Empty, fault cleared

    Outputs tick(const Measurement& m);  // exactly once per second

    Stage stage() const { return stage_; }
    Fault fault() const { return fault_; }
    bool running() const;
    bool paused() const { return paused_; }
    uint32_t stageSeconds() const { return stageSeconds_; }
    const Results& results() const { return results_; }
    float lastCellV() const { return lastV_; }
    float lastCurrentA() const { return lastI_; }
    float lastTempC() const { return lastT_; }

  private:
    void enter(Stage s);
    void fail(Fault f);
    float cutoffVolts() const { return settings_.cutoffMv / 1000.0f; }
    Outputs chargeTick(const Measurement& m, Stage next);
    Outputs riTick(const Measurement& m);
    Outputs dischargeTick(const Measurement& m);

    ChannelSettings settings_;
    Stage stage_;
    Fault fault_;
    bool paused_;
    uint32_t stageSeconds_;
    uint8_t confirm_;
    uint8_t noCurrent_;
    float vOpen_;
    float lastV_;
    float lastI_;
    float lastT_;
    Results results_;
};

}  // namespace tc
