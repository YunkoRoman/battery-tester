#include <unity.h>
#include "calc.h"

void setUp() {}
void tearDown() {}

void test_duty_for_currents() {
    TEST_ASSERT_EQUAL_UINT8(0, tc::currentToDuty(0));
    TEST_ASSERT_EQUAL_UINT8(50, tc::currentToDuty(200));  // 0.2 * 255 / 1.0204 = 49.98
    TEST_ASSERT_EQUAL_UINT8(125, tc::currentToDuty(500));
    TEST_ASSERT_EQUAL_UINT8(250, tc::currentToDuty(1000));
    TEST_ASSERT_EQUAL_UINT8(255, tc::currentToDuty(2000));
}

void test_internal_resistance() {
    TEST_ASSERT_EQUAL_UINT16(100, tc::internalResistanceMilliOhm(4.10f, 4.05f, 0.5f));
    TEST_ASSERT_EQUAL_UINT16(0, tc::internalResistanceMilliOhm(4.00f, 4.00f, 0.5f));
    TEST_ASSERT_EQUAL_UINT16(0, tc::internalResistanceMilliOhm(4.10f, 4.00f, 0.01f));
    TEST_ASSERT_EQUAL_UINT16(65535, tc::internalResistanceMilliOhm(4.10f, 0.10f, 0.05f));
}

void test_lipo_percent() {
    TEST_ASSERT_EQUAL_UINT8(100, tc::liPoPercent(4.30f));
    TEST_ASSERT_EQUAL_UINT8(100, tc::liPoPercent(4.20f));
    TEST_ASSERT_EQUAL_UINT8(40, tc::liPoPercent(3.84f));
    TEST_ASSERT_EQUAL_UINT8(0, tc::liPoPercent(2.00f));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_duty_for_currents);
    RUN_TEST(test_internal_resistance);
    RUN_TEST(test_lipo_percent);
    return UNITY_END();
}
