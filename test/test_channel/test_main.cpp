#include <unity.h>
#include "calc.h"
#include "channel.h"

using namespace tc;

static bool sawRelayAndLoad = false;

static Measurement M(float v, float i = 0.0f, float t = 25.0f, bool done = false, bool power = true) {
    Measurement m;
    m.cellV = v;
    m.currentA = i;
    m.tempC = t;
    m.chargeDone = done;
    m.powerOk = power;
    return m;
}

static Outputs run(Channel& ch, const Measurement& m, uint32_t ticks) {
    Outputs o = {0, false};
    for (uint32_t k = 0; k < ticks; k++) {
        o = ch.tick(m);
        if (o.relay && o.duty > 0) sawRelayAndLoad = true;
    }
    return o;
}

static void withMode(Channel& ch, Mode mode) {
    ChannelSettings s = defaultSettings();
    s.mode = mode;
    ch.configure(s);
}

// DISCHARGE mode, through a passing Ri test (100 mΩ at 0.5 A).
static void startDischarge(Channel& ch) {
    withMode(ch, Mode::Discharge);
    ch.tick(M(4.10f));
    TEST_ASSERT_TRUE(ch.start());
    run(ch, M(4.10f), 1);        // open-circuit voltage
    run(ch, M(4.06f, 0.5f), 1);  // settling under load
    run(ch, M(4.05f, 0.5f), 1);  // measure
    TEST_ASSERT_EQUAL(Stage::Discharge, ch.stage());
}

void setUp() { sawRelayAndLoad = false; }
void tearDown() {}

void test_insert_cell_goes_idle_and_back_empty() {
    Channel ch;
    ch.tick(M(0.0f));
    TEST_ASSERT_EQUAL(Stage::Empty, ch.stage());
    ch.tick(M(3.9f));
    TEST_ASSERT_EQUAL(Stage::Idle, ch.stage());
    ch.tick(M(0.1f));
    TEST_ASSERT_EQUAL(Stage::Empty, ch.stage());
}

void test_start_refused_without_cell() {
    Channel ch;
    ch.tick(M(0.0f));
    TEST_ASSERT_FALSE(ch.start());
    TEST_ASSERT_EQUAL(Stage::Empty, ch.stage());
}

void test_dead_cell_rejected() {
    Channel ch;
    ch.tick(M(1.5f));
    TEST_ASSERT_TRUE(ch.start());
    TEST_ASSERT_EQUAL(Stage::Fault, ch.stage());
    TEST_ASSERT_EQUAL(Fault::Dead, ch.fault());
    Outputs o = ch.tick(M(1.5f));
    TEST_ASSERT_FALSE(o.relay);
    TEST_ASSERT_EQUAL_UINT8(0, o.duty);
}

void test_full_cycle() {
    Channel ch;
    ch.tick(M(3.7f));
    TEST_ASSERT_TRUE(ch.start());
    TEST_ASSERT_EQUAL(Stage::Charge, ch.stage());

    Outputs o = run(ch, M(3.9f), 10);
    TEST_ASSERT_TRUE(o.relay);
    TEST_ASSERT_EQUAL_UINT8(0, o.duty);

    run(ch, M(4.2f, 0.0f, 25.0f, true), kChargeDoneConfirm);
    TEST_ASSERT_EQUAL(Stage::RestCharged, ch.stage());

    run(ch, M(4.15f), kRestSeconds);
    TEST_ASSERT_EQUAL(Stage::RiTest, ch.stage());

    o = run(ch, M(4.10f), 1);
    TEST_ASSERT_EQUAL_UINT8(currentToDuty(500), o.duty);
    TEST_ASSERT_FALSE(o.relay);
    run(ch, M(4.06f, 0.5f), 1);
    run(ch, M(4.05f, 0.5f), 1);
    TEST_ASSERT_EQUAL(Stage::Discharge, ch.stage());
    TEST_ASSERT_EQUAL_UINT16(100, ch.results().riMilliOhm);

    run(ch, M(3.7f, 0.5f), 400);
    run(ch, M(2.95f, 0.5f), kCutoffConfirm);
    TEST_ASSERT_EQUAL(Stage::RestDischarged, ch.stage());
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 405 * 0.5f * 1000.0f / 3600.0f, ch.results().mAh);
    TEST_ASSERT_EQUAL_UINT32(405, ch.results().dischargeSeconds);

    run(ch, M(3.3f), kRestSeconds);
    TEST_ASSERT_EQUAL(Stage::Recharge, ch.stage());
    run(ch, M(4.2f, 0.0f, 25.0f, true), kChargeDoneConfirm);
    TEST_ASSERT_EQUAL(Stage::Done, ch.stage());

    o = ch.tick(M(4.2f));
    TEST_ASSERT_FALSE(o.relay);
    TEST_ASSERT_EQUAL_UINT8(0, o.duty);
    TEST_ASSERT_FALSE(sawRelayAndLoad);
}

void test_discharge_mode_ends_done() {
    Channel ch;
    startDischarge(ch);
    run(ch, M(3.7f, 0.5f), 400);
    run(ch, M(2.9f, 0.5f), kCutoffConfirm);
    TEST_ASSERT_EQUAL(Stage::Done, ch.stage());
}

void test_charge_mode_ends_done() {
    Channel ch;
    withMode(ch, Mode::Charge);
    ch.tick(M(3.7f));
    TEST_ASSERT_TRUE(ch.start());
    TEST_ASSERT_EQUAL(Stage::Charge, ch.stage());
    run(ch, M(4.2f, 0.0f, 25.0f, true), kChargeDoneConfirm);
    TEST_ASSERT_EQUAL(Stage::Done, ch.stage());
}

void test_bad_ri() {
    Channel ch;
    withMode(ch, Mode::Discharge);
    ch.tick(M(4.10f));
    ch.start();
    run(ch, M(4.10f), 1);
    run(ch, M(3.95f, 0.5f), 1);
    run(ch, M(3.90f, 0.5f), 1);  // 400 mΩ
    TEST_ASSERT_EQUAL(Fault::BadRi, ch.fault());
    TEST_ASSERT_EQUAL_UINT16(400, ch.results().riMilliOhm);
}

void test_ri_load_below_cutoff_is_sag() {
    Channel ch;
    withMode(ch, Mode::Discharge);
    ch.tick(M(3.10f));
    ch.start();
    run(ch, M(3.10f), 1);
    run(ch, M(3.02f, 0.5f), 1);
    run(ch, M(3.00f, 0.5f), 1);  // 200 mΩ is fine, but already at cutoff
    TEST_ASSERT_EQUAL(Fault::Sag, ch.fault());
}

void test_early_cutoff_is_sag() {
    Channel ch;
    startDischarge(ch);
    run(ch, M(3.5f, 0.5f), 100);
    run(ch, M(2.9f, 0.5f), kCutoffConfirm);
    TEST_ASSERT_EQUAL(Fault::Sag, ch.fault());
}

void test_cutoff_needs_consecutive_samples() {
    Channel ch;
    startDischarge(ch);
    run(ch, M(3.7f, 0.5f), 400);
    run(ch, M(2.9f, 0.5f), kCutoffConfirm - 1);
    run(ch, M(3.1f, 0.5f), 1);
    run(ch, M(2.9f, 0.5f), kCutoffConfirm - 1);
    TEST_ASSERT_EQUAL(Stage::Discharge, ch.stage());
}

void test_hot_while_charging() {
    Channel ch;
    ch.tick(M(3.7f));
    ch.start();
    run(ch, M(3.9f, 0.0f, 45.0f), 5);  // exactly on the threshold: still fine
    TEST_ASSERT_EQUAL(Stage::Charge, ch.stage());
    run(ch, M(3.9f, 0.0f, 45.5f), 1);
    TEST_ASSERT_EQUAL(Fault::Hot, ch.fault());
}

void test_warm_discharge_ok_hot_fails() {
    Channel ch;
    startDischarge(ch);
    run(ch, M(3.7f, 0.5f, 50.0f), 10);
    TEST_ASSERT_EQUAL(Stage::Discharge, ch.stage());
    run(ch, M(3.7f, 0.5f, 56.0f), 1);
    TEST_ASSERT_EQUAL(Fault::Hot, ch.fault());
}

void test_removed_while_running() {
    Channel ch;
    startDischarge(ch);
    Outputs o = run(ch, M(0.2f), 1);
    TEST_ASSERT_EQUAL(Fault::Removed, ch.fault());
    TEST_ASSERT_EQUAL_UINT8(0, o.duty);
}

void test_ntc_error_only_when_running() {
    Channel ch;
    ch.tick(M(3.7f, 0.0f, kTempInvalid));
    TEST_ASSERT_EQUAL(Stage::Idle, ch.stage());
    ch.start();
    ch.tick(M(3.7f, 0.0f, kTempInvalid));
    TEST_ASSERT_EQUAL(Fault::SensorError, ch.fault());
}

void test_no_power_pauses_discharge() {
    Channel ch;
    startDischarge(ch);
    run(ch, M(3.7f, 0.5f), 10);
    float mAh = ch.results().mAh;
    uint32_t seconds = ch.results().dischargeSeconds;
    Outputs o = run(ch, M(3.7f, 0.0f, 25.0f, false, false), 30);
    TEST_ASSERT_TRUE(ch.paused());
    TEST_ASSERT_EQUAL(Stage::Discharge, ch.stage());
    TEST_ASSERT_EQUAL_UINT8(0, o.duty);
    TEST_ASSERT_FALSE(o.relay);
    TEST_ASSERT_EQUAL_FLOAT(mAh, ch.results().mAh);
    TEST_ASSERT_EQUAL_UINT32(seconds, ch.results().dischargeSeconds);
    o = run(ch, M(3.7f, 0.5f), 1);
    TEST_ASSERT_FALSE(ch.paused());
    TEST_ASSERT_EQUAL_UINT8(currentToDuty(500), o.duty);
}

void test_no_power_restarts_ri_test() {
    Channel ch;
    withMode(ch, Mode::Discharge);
    ch.tick(M(4.10f));
    ch.start();
    run(ch, M(4.10f), 1);
    run(ch, M(4.06f, 0.5f), 1);
    run(ch, M(4.00f, 0.0f, 25.0f, false, false), 1);
    TEST_ASSERT_TRUE(ch.paused());
    TEST_ASSERT_EQUAL(Stage::RiTest, ch.stage());
    run(ch, M(4.10f), 1);
    run(ch, M(4.06f, 0.5f), 1);
    run(ch, M(4.05f, 0.5f), 1);
    TEST_ASSERT_EQUAL(Stage::Discharge, ch.stage());
    TEST_ASSERT_EQUAL_UINT16(100, ch.results().riMilliOhm);
}

void test_start_without_power_stays_paused() {
    Channel ch;
    ch.tick(M(3.8f, 0.0f, 25.0f, false, false));
    TEST_ASSERT_EQUAL(Stage::Idle, ch.stage());
    TEST_ASSERT_TRUE(ch.start());
    Outputs o = ch.tick(M(3.8f, 0.0f, 25.0f, false, false));
    TEST_ASSERT_TRUE(ch.paused());
    TEST_ASSERT_FALSE(o.relay);
    TEST_ASSERT_EQUAL_UINT8(0, o.duty);
}

void test_charge_timeout() {
    Channel ch;
    ch.tick(M(3.7f));
    ch.start();
    run(ch, M(3.9f), kChargeTimeoutSeconds);
    TEST_ASSERT_EQUAL(Fault::ChargeTimeout, ch.fault());
}

void test_no_current_in_discharge() {
    Channel ch;
    startDischarge(ch);
    run(ch, M(3.7f, 0.0f), kNoCurrentConfirm);
    TEST_ASSERT_EQUAL(Fault::NoCurrent, ch.fault());
}

void test_no_current_in_ri_test() {
    Channel ch;
    withMode(ch, Mode::Discharge);
    ch.tick(M(4.10f));
    ch.start();
    run(ch, M(4.10f), 1);
    run(ch, M(4.10f, 0.0f), 2);
    TEST_ASSERT_EQUAL(Fault::NoCurrent, ch.fault());
}

void test_settings_locked_while_running() {
    Channel ch;
    startDischarge(ch);
    ChannelSettings s = defaultSettings();
    s.currentMa = 1000;
    ch.configure(s);
    TEST_ASSERT_EQUAL_UINT16(500, ch.settings().currentMa);
}

void test_stop_turns_everything_off() {
    Channel ch;
    startDischarge(ch);
    ch.stop();
    TEST_ASSERT_EQUAL(Stage::Idle, ch.stage());
    Outputs o = ch.tick(M(3.7f));
    TEST_ASSERT_EQUAL_UINT8(0, o.duty);
    TEST_ASSERT_FALSE(o.relay);
}

void test_done_survives_cell_swap() {
    Channel ch;
    withMode(ch, Mode::Charge);
    ch.tick(M(3.7f));
    ch.start();
    run(ch, M(4.2f, 0.0f, 25.0f, true), kChargeDoneConfirm);
    TEST_ASSERT_EQUAL(Stage::Done, ch.stage());
    run(ch, M(0.0f), 3);
    TEST_ASSERT_EQUAL(Stage::Done, ch.stage());
    Outputs o = run(ch, M(3.9f), 3);
    TEST_ASSERT_EQUAL(Stage::Done, ch.stage());
    TEST_ASSERT_FALSE(o.relay);
    TEST_ASSERT_EQUAL_UINT8(0, o.duty);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_insert_cell_goes_idle_and_back_empty);
    RUN_TEST(test_start_refused_without_cell);
    RUN_TEST(test_dead_cell_rejected);
    RUN_TEST(test_full_cycle);
    RUN_TEST(test_discharge_mode_ends_done);
    RUN_TEST(test_charge_mode_ends_done);
    RUN_TEST(test_bad_ri);
    RUN_TEST(test_ri_load_below_cutoff_is_sag);
    RUN_TEST(test_early_cutoff_is_sag);
    RUN_TEST(test_cutoff_needs_consecutive_samples);
    RUN_TEST(test_hot_while_charging);
    RUN_TEST(test_warm_discharge_ok_hot_fails);
    RUN_TEST(test_removed_while_running);
    RUN_TEST(test_ntc_error_only_when_running);
    RUN_TEST(test_no_power_pauses_discharge);
    RUN_TEST(test_no_power_restarts_ri_test);
    RUN_TEST(test_start_without_power_stays_paused);
    RUN_TEST(test_charge_timeout);
    RUN_TEST(test_no_current_in_discharge);
    RUN_TEST(test_no_current_in_ri_test);
    RUN_TEST(test_settings_locked_while_running);
    RUN_TEST(test_stop_turns_everything_off);
    RUN_TEST(test_done_survives_cell_swap);
    return UNITY_END();
}
