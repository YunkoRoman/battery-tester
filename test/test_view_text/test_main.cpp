#include <string.h>
#include <unity.h>
#include "view_text.h"

using namespace tc;

static Measurement M(float v, float i = 0.0f, float t = 25.0f, bool done = false) {
    Measurement m;
    m.cellV = v;
    m.currentA = i;
    m.tempC = t;
    m.chargeDone = done;
    m.powerOk = true;
    return m;
}

void setUp() {}
void tearDown() {}

void test_fmt_fixed() {
    char b[12];
    fmtFixed(b, sizeof b, 372, 2);
    TEST_ASSERT_EQUAL_STRING("3.72", b);
    fmtFixed(b, sizeof b, 5, 2);
    TEST_ASSERT_EQUAL_STRING("0.05", b);
    fmtFixed(b, sizeof b, -15, 1);
    TEST_ASSERT_EQUAL_STRING("-1.5", b);
    fmtFixed(b, sizeof b, 1234, 0);
    TEST_ASSERT_EQUAL_STRING("1234", b);
}

void test_hms() {
    char b[10];
    fmtHms(b, sizeof b, 3725);
    TEST_ASSERT_EQUAL_STRING("01:02:05", b);
}

void test_header_right_aligns_battery() {
    char b[kLineBuf];
    headerLine(b, "TESTER 4x18650", 87);
    TEST_ASSERT_EQUAL_STRING("TESTER 4x18650   NANIT 87%", b);
    headerLine(b, "CH1 FAULT CHG TIMEOUT XXXXXXX", 100);
    TEST_ASSERT_TRUE(strlen(b) <= kLineChars);
}

void test_overview_rows() {
    char b[kLineBuf];
    Channel empty;
    empty.tick(M(0.0f));
    overviewRow(b, 1, empty);
    TEST_ASSERT_EQUAL_STRING("2 ---  no cell", b);
    Channel idle;
    idle.tick(M(3.72f));
    overviewRow(b, 0, idle);
    TEST_ASSERT_EQUAL_STRING("1 IDLE 3.72V 0.00A 0mAh", b);
}

void test_channel_lines() {
    char b[kLineBuf];
    Channel ch;
    ch.tick(M(3.70f));
    channelLine(b, 0, ch);
    TEST_ASSERT_EQUAL_STRING("V     3.70 V", b);
    channelLine(b, 2, ch);
    TEST_ASSERT_EQUAL_STRING("T     25 C", b);
    channelLine(b, 3, ch);
    TEST_ASSERT_EQUAL_STRING("CAP   0 mAh 0.00Wh", b);
    channelLine(b, 4, ch);
    TEST_ASSERT_EQUAL_STRING("Ri    --", b);
    ch.tick(M(3.70f, 0.0f, kTempInvalid));
    channelLine(b, 2, ch);
    TEST_ASSERT_EQUAL_STRING("T     --", b);
}

void test_settings_lines() {
    char b[kLineBuf];
    ChannelSettings s = defaultSettings();
    settingsLine(b, 0, s, 0, false, false);
    TEST_ASSERT_EQUAL_STRING("> CURRENT   0.50 A", b);
    settingsLine(b, 0, s, 0, true, false);
    TEST_ASSERT_EQUAL_STRING("* CURRENT   0.50 A", b);
    settingsLine(b, 1, s, 0, false, false);
    TEST_ASSERT_EQUAL_STRING("  CUTOFF    3.00 V", b);
    settingsLine(b, 2, s, 0, false, false);
    TEST_ASSERT_EQUAL_STRING("  MODE      FULL", b);
    settingsLine(b, 3, s, 0, false, true);
    TEST_ASSERT_EQUAL_STRING("  STOP", b);
}

void test_service_lines() {
    char b[kLineBuf];
    serviceRow(b, 0, M(3.72f, 0.20f, 25.0f, true), ServiceOut::Load);
    TEST_ASSERT_EQUAL_STRING("1 3.72V 0.20A 25C D LOAD", b);
    serviceRow(b, 3, M(0.0f, 0.0f, kTempInvalid), ServiceOut::Off);
    TEST_ASSERT_EQUAL_STRING("4 0.00V 0.00A --C - OFF", b);
    serviceFooter(b, 5.02f, true);
    TEST_ASSERT_EQUAL_STRING("BUS 5.02V  FAN ON", b);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_fmt_fixed);
    RUN_TEST(test_hms);
    RUN_TEST(test_header_right_aligns_battery);
    RUN_TEST(test_overview_rows);
    RUN_TEST(test_channel_lines);
    RUN_TEST(test_settings_lines);
    RUN_TEST(test_service_lines);
    return UNITY_END();
}
