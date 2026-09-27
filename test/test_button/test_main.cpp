#include <unity.h>
#include "button.h"

using tc::Button;
using tc::Press;

void setUp() {}
void tearDown() {}

void test_short_press() {
    Button b;
    TEST_ASSERT_EQUAL(Press::None, b.update(true, 0));
    TEST_ASSERT_EQUAL(Press::None, b.update(true, 40));
    TEST_ASSERT_EQUAL(Press::None, b.update(false, 100));
    TEST_ASSERT_EQUAL(Press::Short, b.update(false, 140));
    TEST_ASSERT_EQUAL(Press::None, b.update(false, 200));
}

void test_bounce_is_ignored() {
    Button b;
    b.update(true, 0);
    b.update(false, 10);
    b.update(true, 20);
    TEST_ASSERT_EQUAL(Press::None, b.update(false, 25));
    TEST_ASSERT_EQUAL(Press::None, b.update(false, 100));
}

void test_long_press_fires_once() {
    Button b;
    b.update(true, 0);
    b.update(true, 40);
    TEST_ASSERT_EQUAL(Press::Long, b.update(true, 850));
    TEST_ASSERT_EQUAL(Press::None, b.update(true, 900));
    b.update(false, 950);
    TEST_ASSERT_EQUAL(Press::None, b.update(false, 990));
}

void test_press_across_millis_rollover() {
    Button b;
    const uint32_t t0 = 0xFFFFFFF0UL;
    b.update(true, t0);
    b.update(true, t0 + 40);  // wraps past zero
    b.update(false, t0 + 100);
    TEST_ASSERT_EQUAL(Press::Short, b.update(false, t0 + 140));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_short_press);
    RUN_TEST(test_bounce_is_ignored);
    RUN_TEST(test_long_press_fires_once);
    RUN_TEST(test_press_across_millis_rollover);
    return UNITY_END();
}
