#include <unity.h>
#include "power.h"

using tc::PowerMonitor;

void setUp() {}
void tearDown() {}

static bool feed(PowerMonitor& p, float volts, uint8_t times) {
    bool ok = false;
    for (uint8_t k = 0; k < times; k++) ok = p.update(volts);
    return ok;
}

void test_needs_consecutive_good_samples_after_boot() {
    PowerMonitor p;
    TEST_ASSERT_FALSE(feed(p, 5.0f, tc::kPowerResumeTicks - 1));
    TEST_ASSERT_TRUE(p.update(5.0f));
}

void test_drops_immediately_below_threshold() {
    PowerMonitor p;
    feed(p, 5.0f, tc::kPowerResumeTicks);
    TEST_ASSERT_FALSE(p.update(4.4f));
}

void test_hysteresis_keeps_power_between_thresholds() {
    PowerMonitor p;
    feed(p, 5.0f, tc::kPowerResumeTicks);
    TEST_ASSERT_TRUE(feed(p, 4.6f, 10));
}

void test_sagging_bus_does_not_resume() {
    PowerMonitor p;
    feed(p, 5.0f, tc::kPowerResumeTicks);
    p.update(4.4f);
    TEST_ASSERT_FALSE(feed(p, 4.6f, 10));  // below the resume threshold
}

void test_resume_needs_an_unbroken_run() {
    PowerMonitor p;
    p.update(4.8f);
    p.update(4.8f);
    p.update(4.4f);  // breaks the run
    p.update(4.8f);
    TEST_ASSERT_FALSE(p.update(4.8f));
    TEST_ASSERT_TRUE(p.update(4.8f));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_needs_consecutive_good_samples_after_boot);
    RUN_TEST(test_drops_immediately_below_threshold);
    RUN_TEST(test_hysteresis_keeps_power_between_thresholds);
    RUN_TEST(test_sagging_bus_does_not_resume);
    RUN_TEST(test_resume_needs_an_unbroken_run);
    return UNITY_END();
}
