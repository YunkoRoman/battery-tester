#include "ui_model.h"

namespace tc {

namespace {

uint8_t wrap(uint8_t value, int8_t dir, uint8_t count) {
    return (uint8_t)((value + count + dir) % count);
}

UiResult handleService(UiState& ui, Key key, int8_t dir) {
    UiResult r = {UiAction::None, ui.ch};
    if (key == Key::OkLong) {
        for (uint8_t c = 0; c < kChannels; c++) ui.svc[c] = ServiceOut::Off;
        ui.svcFan = false;
        ui.screen = Screen::Overview;
        r.action = UiAction::ExitService;
        return r;
    }
    if (dir != 0) {
        ui.svcRow = wrap(ui.svcRow, dir, kChannels + 1);
        return r;
    }
    if (ui.svcRow < kChannels) {
        ServiceOut& o = ui.svc[ui.svcRow];
        o = o == ServiceOut::Off ? ServiceOut::Relay : (o == ServiceOut::Relay ? ServiceOut::Load : ServiceOut::Off);
    } else {
        ui.svcFan = !ui.svcFan;
    }
    return r;
}

UiResult handleSettings(UiState& ui, int8_t dir, ChannelSettings* settings, const bool* running) {
    UiResult r = {UiAction::None, ui.ch};
    ChannelSettings& s = settings[ui.ch];
    SettingsItem item = (SettingsItem)ui.item;
    if (ui.editing) {
        if (dir == 0) {
            ui.editing = false;
            return r;
        }
        if (item == SettingsItem::Current) stepCurrent(s, dir);
        else if (item == SettingsItem::Cutoff) stepCutoff(s, dir);
        else stepMode(s, dir);
        r.action = UiAction::SettingsChanged;
        return r;
    }
    if (dir != 0) {
        ui.item = wrap(ui.item, dir, (uint8_t)SettingsItem::Count);
        return r;
    }
    switch (item) {
        case SettingsItem::Current:
        case SettingsItem::Cutoff:
        case SettingsItem::Mode:
            if (!running[ui.ch]) ui.editing = true;
            break;
        case SettingsItem::StartStop:
            r.action = running[ui.ch] ? UiAction::Stop : UiAction::Start;
            break;
        case SettingsItem::Back:
            ui.screen = Screen::ChannelView;
            break;
        default:
            break;
    }
    return r;
}

}  // namespace

UiState initialUi(bool service) {
    UiState u;
    u.screen = service ? Screen::Service : Screen::Overview;
    u.ch = 0;
    u.item = 0;
    u.editing = false;
    for (uint8_t c = 0; c < kChannels; c++) u.svc[c] = ServiceOut::Off;
    u.svcFan = false;
    u.svcRow = 0;
    return u;
}

UiResult handleKey(UiState& ui, Key key, ChannelSettings* settings, const bool* running) {
    int8_t dir = key == Key::Left ? -1 : (key == Key::Right ? 1 : 0);
    if (ui.screen == Screen::Service) return handleService(ui, key, dir);

    UiResult r = {UiAction::None, ui.ch};
    if (key == Key::OkLong) {
        ui.screen = Screen::Overview;
        ui.editing = false;
        return r;
    }
    switch (ui.screen) {
        case Screen::Overview:
        case Screen::ChannelView:
            if (dir != 0) {
                ui.ch = wrap(ui.ch, dir, kChannels);
                r.ch = ui.ch;
            } else if (ui.screen == Screen::Overview) {
                ui.screen = Screen::ChannelView;
            } else {
                ui.screen = Screen::Settings;
                ui.item = 0;
                ui.editing = false;
            }
            return r;
        case Screen::Settings:
            return handleSettings(ui, dir, settings, running);
        default:
            return r;
    }
}

}  // namespace tc
