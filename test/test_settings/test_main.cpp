#include <string.h>
#include <unity.h>
#include "settings.h"

using namespace tc;

void setUp() {}
void tearDown() {}

void test_defaults() {
    ChannelSettings s = defaultSettings();
    TEST_ASSERT_EQUAL_UINT16(500, s.currentMa);
    TEST_ASSERT_EQUAL_UINT16(3000, s.cutoffMv);
    TEST_ASSERT_EQUAL(Mode::Full, s.mode);
}

void test_current_steps_and_clamps() {
    ChannelSettings s = defaultSettings();
    stepCurrent(s, 1);
    TEST_ASSERT_EQUAL_UINT16(550, s.currentMa);
    s.currentMa = kCurrentMaxMa;
    stepCurrent(s, 1);
    TEST_ASSERT_EQUAL_UINT16(1000, s.currentMa);
    s.currentMa = kCurrentMinMa;
    stepCurrent(s, -1);
    TEST_ASSERT_EQUAL_UINT16(100, s.currentMa);
}

void test_cutoff_clamps() {
    ChannelSettings s = defaultSettings();
    s.cutoffMv = kCutoffMaxMv;
    stepCutoff(s, 1);
    TEST_ASSERT_EQUAL_UINT16(3300, s.cutoffMv);
    s.cutoffMv = kCutoffMinMv;
    stepCutoff(s, -1);
    TEST_ASSERT_EQUAL_UINT16(2800, s.cutoffMv);
}

void test_mode_wraps() {
    ChannelSettings s = defaultSettings();
    stepMode(s, -1);
    TEST_ASSERT_EQUAL(Mode::Charge, s.mode);
    stepMode(s, 1);
    TEST_ASSERT_EQUAL(Mode::Full, s.mode);
}

void test_roundtrip() {
    ChannelSettings a[kChannels], b[kChannels];
    for (uint8_t c = 0; c < kChannels; c++) {
        a[c].currentMa = 100 + 300 * c;
        a[c].cutoffMv = 2800 + 100 * c;
        a[c].mode = (Mode)(c % 3);
    }
    uint8_t blob[kSettingsBlobSize];
    packSettings(a, blob);
    TEST_ASSERT_TRUE(unpackSettings(blob, b));
    for (uint8_t c = 0; c < kChannels; c++) {
        TEST_ASSERT_EQUAL_UINT16(a[c].currentMa, b[c].currentMa);
        TEST_ASSERT_EQUAL_UINT16(a[c].cutoffMv, b[c].cutoffMv);
        TEST_ASSERT_EQUAL(a[c].mode, b[c].mode);
    }
}

void test_blank_eeprom_rejected() {
    uint8_t blob[kSettingsBlobSize];
    memset(blob, 0xFF, sizeof blob);
    ChannelSettings s[kChannels];
    TEST_ASSERT_FALSE(unpackSettings(blob, s));
}

void test_corrupt_crc_rejected() {
    ChannelSettings a[kChannels];
    for (uint8_t c = 0; c < kChannels; c++) a[c] = defaultSettings();
    uint8_t blob[kSettingsBlobSize];
    packSettings(a, blob);
    blob[3] ^= 0x01;
    TEST_ASSERT_FALSE(unpackSettings(blob, a));
}

void test_out_of_range_rejected() {
    ChannelSettings a[kChannels];
    for (uint8_t c = 0; c < kChannels; c++) a[c] = defaultSettings();
    uint8_t blob[kSettingsBlobSize];
    packSettings(a, blob);
    blob[2] = 0xD0;  // channel 1 current = 2000 mA
    blob[3] = 0x07;
    blob[kSettingsBlobSize - 1] = crc8(blob, kSettingsBlobSize - 1);
    TEST_ASSERT_FALSE(unpackSettings(blob, a));
}

void test_unpack_failure_leaves_output_untouched() {
    ChannelSettings s[kChannels];
    for (uint8_t c = 0; c < kChannels; c++) s[c] = defaultSettings();
    s[0].currentMa = 750;
    uint8_t blob[kSettingsBlobSize];
    memset(blob, 0xFF, sizeof blob);
    unpackSettings(blob, s);
    TEST_ASSERT_EQUAL_UINT16(750, s[0].currentMa);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_defaults);
    RUN_TEST(test_current_steps_and_clamps);
    RUN_TEST(test_cutoff_clamps);
    RUN_TEST(test_mode_wraps);
    RUN_TEST(test_roundtrip);
    RUN_TEST(test_blank_eeprom_rejected);
    RUN_TEST(test_corrupt_crc_rejected);
    RUN_TEST(test_out_of_range_rejected);
    RUN_TEST(test_unpack_failure_leaves_output_untouched);
    return UNITY_END();
}
