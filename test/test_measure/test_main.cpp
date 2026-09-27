#include <unity.h>
#include "measure.h"

void setUp() {}
void tearDown() {}

void test_adc_full_scale_is_5v() {
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 5.0f, tc::adcToVolts(1023.0f));
}

void test_shunt_1v_is_1a() {
    TEST_ASSERT_FLOAT_WITHIN(0.005f, 1.0f, tc::shuntAmps(204.6f));
}

void test_bus_ok_threshold() {
    TEST_ASSERT_TRUE(tc::busOk(511.5f));    // 2.50 V at A7 → 5.00 V bus
    TEST_ASSERT_FALSE(tc::busOk(450.0f));   // 4.40 V bus
    TEST_ASSERT_FALSE(tc::busOk(0.0f));
}

void test_ntc_25c_at_midscale() {
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 25.0f, tc::ntcCelsius(511.5f));
}

void test_ntc_50c() {
    // B3950: R(50 °C) ≈ 3588 Ω → 1023 * 3588 / 13588 ≈ 270.1 counts
    TEST_ASSERT_FLOAT_WITHIN(0.7f, 50.0f, tc::ntcCelsius(270.1f));
}

void test_ntc_open_or_short_is_invalid() {
    TEST_ASSERT_EQUAL_FLOAT(tc::kTempInvalid, tc::ntcCelsius(1023.0f));
    TEST_ASSERT_EQUAL_FLOAT(tc::kTempInvalid, tc::ntcCelsius(0.0f));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_adc_full_scale_is_5v);
    RUN_TEST(test_shunt_1v_is_1a);
    RUN_TEST(test_bus_ok_threshold);
    RUN_TEST(test_ntc_25c_at_midscale);
    RUN_TEST(test_ntc_50c);
    RUN_TEST(test_ntc_open_or_short_is_invalid);
    return UNITY_END();
}
