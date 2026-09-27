#include <unity.h>
#include "calc.h"
#include "service.h"

using namespace tc;

static Measurement M(float v, float t = 25.0f) {
    Measurement m;
    m.cellV = v;
    m.currentA = 0.0f;
    m.tempC = t;
    m.chargeDone = false;
    m.powerOk = true;
    return m;
}

void setUp() {}
void tearDown() {}

void test_load_runs_on_healthy_cell() {
    ServiceOut o = ServiceOut::Load;
    Outputs out = serviceOutputs(o, M(3.7f));
    TEST_ASSERT_EQUAL_UINT8(currentToDuty(kServiceLoadMa), out.duty);
    TEST_ASSERT_FALSE(out.relay);
    TEST_ASSERT_EQUAL(ServiceOut::Load, o);
}

void test_load_allowed_before_ntc_is_fitted() {
    ServiceOut o = ServiceOut::Load;
    Outputs out = serviceOutputs(o, M(3.7f, kTempInvalid));
    TEST_ASSERT_TRUE(out.duty > 0);
}

void test_load_drops_at_low_voltage() {
    ServiceOut o = ServiceOut::Load;
    Outputs out = serviceOutputs(o, M(2.79f));
    TEST_ASSERT_EQUAL_UINT8(0, out.duty);
    TEST_ASSERT_EQUAL(ServiceOut::Off, o);
}

void test_load_drops_when_hot() {
    ServiceOut o = ServiceOut::Load;
    Outputs out = serviceOutputs(o, M(3.7f, 56.0f));
    TEST_ASSERT_EQUAL_UINT8(0, out.duty);
    TEST_ASSERT_EQUAL(ServiceOut::Off, o);
}

void test_relay_drops_when_hot() {
    ServiceOut o = ServiceOut::Relay;
    Outputs out = serviceOutputs(o, M(3.9f, 46.0f));
    TEST_ASSERT_FALSE(out.relay);
    TEST_ASSERT_EQUAL(ServiceOut::Off, o);
}

void test_relay_runs_when_cool() {
    ServiceOut o = ServiceOut::Relay;
    Outputs out = serviceOutputs(o, M(3.9f, 30.0f));
    TEST_ASSERT_TRUE(out.relay);
    TEST_ASSERT_EQUAL_UINT8(0, out.duty);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_load_runs_on_healthy_cell);
    RUN_TEST(test_load_allowed_before_ntc_is_fitted);
    RUN_TEST(test_load_drops_at_low_voltage);
    RUN_TEST(test_load_drops_when_hot);
    RUN_TEST(test_relay_drops_when_hot);
    RUN_TEST(test_relay_runs_when_cool);
    return UNITY_END();
}
