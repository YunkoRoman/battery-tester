#include <Arduino.h>
#include <EEPROM.h>
#include <NanitLib.h>

#include "board_io.h"
#include "button.h"
#include "calc.h"
#include "channel.h"
#include "display.h"
#include "measure.h"
#include "power.h"
#include "service.h"
#include "settings.h"
#include "ui_model.h"

namespace {

tc::Channel channels[tc::kChannels];
tc::ChannelSettings settings[tc::kChannels];
tc::Measurement meas[tc::kChannels];
tc::UiState ui;
tc::Button buttons[3];
tc::PowerMonitor power;
bool powerOk = false;
float busV = 0.0f;
uint32_t lastTickMs = 0;
bool dirty = true;

const int kEepromAddr = 0;
const tc::Outputs kOff = {0, false};

void loadSettings() {
    uint8_t blob[tc::kSettingsBlobSize];
    for (uint8_t k = 0; k < tc::kSettingsBlobSize; k++) blob[k] = EEPROM.read(kEepromAddr + k);
    if (!tc::unpackSettings(blob, settings)) {
        for (uint8_t c = 0; c < tc::kChannels; c++) settings[c] = tc::defaultSettings();
    }
}

void saveSettings() {
    uint8_t blob[tc::kSettingsBlobSize];
    tc::packSettings(settings, blob);
    for (uint8_t k = 0; k < tc::kSettingsBlobSize; k++) EEPROM.update(kEepromAddr + k, blob[k]);
}

bool drawsPower(const tc::Channel& ch) {
    if (ch.paused()) return false;
    tc::Stage s = ch.stage();
    return s == tc::Stage::Charge || s == tc::Stage::Recharge || s == tc::Stage::RiTest || s == tc::Stage::Discharge;
}

void serviceOutputs() {
    for (uint8_t c = 0; c < tc::kChannels; c++) io::apply(c, tc::serviceOutputs(ui.svc[c], meas[c]));
    io::setFan(ui.svcFan);
}

void logChannel(uint8_t c) {
    const tc::Channel& ch = channels[c];
    Serial.print(c + 1);
    Serial.print(',');
    Serial.print((int)ch.stage());
    Serial.print(',');
    Serial.print(ch.lastCellV(), 3);
    Serial.print(',');
    Serial.print(ch.lastCurrentA(), 3);
    Serial.print(',');
    Serial.print(ch.lastTempC(), 1);
    Serial.print(',');
    Serial.println(ch.results().mAh, 1);
}

void tick() {
    busV = io::busVolts();
    powerOk = power.update(busV);
    io::measureAll(meas, powerOk);
    if (ui.screen == tc::Screen::Service) {
        serviceOutputs();
        return;
    }
    bool fan = false;
    for (uint8_t c = 0; c < tc::kChannels; c++) {
        io::apply(c, channels[c].tick(meas[c]));
        fan = fan || drawsPower(channels[c]);
        if (channels[c].running()) logChannel(c);
    }
    io::setFan(fan);
}

void onKey(tc::Key key) {
    bool running[tc::kChannels];
    for (uint8_t c = 0; c < tc::kChannels; c++) running[c] = channels[c].running();
    tc::UiResult r = tc::handleKey(ui, key, settings, running);
    switch (r.action) {
        case tc::UiAction::Start:
            channels[r.ch].start();
            break;
        case tc::UiAction::Stop:
            channels[r.ch].stop();
            io::apply(r.ch, kOff);
            break;
        case tc::UiAction::SettingsChanged:
            channels[r.ch].configure(settings[r.ch]);
            saveSettings();
            break;
        case tc::UiAction::ExitService:
            for (uint8_t c = 0; c < tc::kChannels; c++) io::apply(c, kOff);
            io::setFan(false);
            break;
        default:
            break;
    }
    dirty = true;
}

void draw() {
    view::Frame f = {&ui, channels, meas, settings, powerOk, busV, tc::liPoPercent(io::nanitBatteryVolts())};
    view::render(f);
    dirty = false;
}

}  // namespace

void setup() {
    io::begin();         // outputs safe-off before anything else
    Nanit_Base_Start();  // TFT init; touches only TFT pins
    Serial.begin(9600);
    loadSettings();
    for (uint8_t c = 0; c < tc::kChannels; c++) channels[c].configure(settings[c]);

    delay(50);
    bool service = io::buttonDown(1);
    while (io::buttonDown(1)) delay(10);  // swallow the boot press so it doesn't toggle row 1
    ui = tc::initialUi(service);

    view::begin();
    lastTickMs = millis();
}

void loop() {
    const uint32_t now = millis();
    static const tc::Key kShortKey[3] = {tc::Key::Left, tc::Key::Ok, tc::Key::Right};
    for (uint8_t b = 0; b < 3; b++) {
        tc::Press p = buttons[b].update(io::buttonDown(b), now);
        if (p == tc::Press::Short) onKey(kShortKey[b]);
        else if (p == tc::Press::Long && b == 1) onKey(tc::Key::OkLong);
    }
    if (now - lastTickMs >= 1000) {
        lastTickMs += 1000;
        tick();
        dirty = true;
    }
    if (dirty) draw();
}
