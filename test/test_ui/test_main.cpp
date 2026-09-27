#include <unity.h>
#include "ui_model.h"

using namespace tc;

static ChannelSettings S[kChannels];
static bool R[kChannels];

void setUp() {
    for (uint8_t c = 0; c < kChannels; c++) {
        S[c] = defaultSettings();
        R[c] = false;
    }
}
void tearDown() {}

static UiState settingsOfChannel0() {
    UiState u = initialUi(false);
    handleKey(u, Key::Ok, S, R);  // overview → channel
    handleKey(u, Key::Ok, S, R);  // channel → settings
    TEST_ASSERT_EQUAL(Screen::Settings, u.screen);
    return u;
}

void test_overview_wraps_and_opens_channel() {
    UiState u = initialUi(false);
    TEST_ASSERT_EQUAL(Screen::Overview, u.screen);
    handleKey(u, Key::Left, S, R);
    TEST_ASSERT_EQUAL_UINT8(3, u.ch);
    handleKey(u, Key::Right, S, R);
    TEST_ASSERT_EQUAL_UINT8(0, u.ch);
    handleKey(u, Key::Ok, S, R);
    TEST_ASSERT_EQUAL(Screen::ChannelView, u.screen);
}

void test_long_ok_goes_home() {
    UiState u = settingsOfChannel0();
    handleKey(u, Key::Ok, S, R);  // start editing current
    TEST_ASSERT_TRUE(u.editing);
    handleKey(u, Key::OkLong, S, R);
    TEST_ASSERT_EQUAL(Screen::Overview, u.screen);
    TEST_ASSERT_FALSE(u.editing);
}

void test_edit_current() {
    UiState u = settingsOfChannel0();
    handleKey(u, Key::Ok, S, R);
    UiResult r = handleKey(u, Key::Right, S, R);
    TEST_ASSERT_EQUAL(UiAction::SettingsChanged, r.action);
    TEST_ASSERT_EQUAL_UINT8(0, r.ch);
    TEST_ASSERT_EQUAL_UINT16(550, S[0].currentMa);
    handleKey(u, Key::Ok, S, R);
    TEST_ASSERT_FALSE(u.editing);
}

void test_edit_locked_while_running() {
    R[0] = true;
    UiState u = settingsOfChannel0();
    handleKey(u, Key::Ok, S, R);
    TEST_ASSERT_FALSE(u.editing);
}

void test_start_and_stop() {
    UiState u = settingsOfChannel0();
    for (uint8_t k = 0; k < 3; k++) handleKey(u, Key::Right, S, R);
    TEST_ASSERT_EQUAL_UINT8((uint8_t)SettingsItem::StartStop, u.item);
    TEST_ASSERT_EQUAL(UiAction::Start, handleKey(u, Key::Ok, S, R).action);
    R[0] = true;
    TEST_ASSERT_EQUAL(UiAction::Stop, handleKey(u, Key::Ok, S, R).action);
}

void test_back_returns_to_channel() {
    UiState u = settingsOfChannel0();
    handleKey(u, Key::Left, S, R);  // wraps to Back
    TEST_ASSERT_EQUAL_UINT8((uint8_t)SettingsItem::Back, u.item);
    handleKey(u, Key::Ok, S, R);
    TEST_ASSERT_EQUAL(Screen::ChannelView, u.screen);
}

void test_service_cycle_and_exit() {
    UiState u = initialUi(true);
    TEST_ASSERT_EQUAL(Screen::Service, u.screen);
    handleKey(u, Key::Ok, S, R);
    TEST_ASSERT_EQUAL(ServiceOut::Relay, u.svc[0]);
    handleKey(u, Key::Ok, S, R);
    TEST_ASSERT_EQUAL(ServiceOut::Load, u.svc[0]);
    handleKey(u, Key::Ok, S, R);
    TEST_ASSERT_EQUAL(ServiceOut::Off, u.svc[0]);
    handleKey(u, Key::Left, S, R);  // wraps to the fan row
    TEST_ASSERT_EQUAL_UINT8(kChannels, u.svcRow);
    handleKey(u, Key::Ok, S, R);
    TEST_ASSERT_TRUE(u.svcFan);
    handleKey(u, Key::Right, S, R);  // row 0
    handleKey(u, Key::Ok, S, R);     // relay on
    UiResult r = handleKey(u, Key::OkLong, S, R);
    TEST_ASSERT_EQUAL(UiAction::ExitService, r.action);
    TEST_ASSERT_EQUAL(Screen::Overview, u.screen);
    TEST_ASSERT_FALSE(u.svcFan);
    TEST_ASSERT_EQUAL(ServiceOut::Off, u.svc[0]);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_overview_wraps_and_opens_channel);
    RUN_TEST(test_long_ok_goes_home);
    RUN_TEST(test_edit_current);
    RUN_TEST(test_edit_locked_while_running);
    RUN_TEST(test_start_and_stop);
    RUN_TEST(test_back_returns_to_channel);
    RUN_TEST(test_service_cycle_and_exit);
    return UNITY_END();
}
