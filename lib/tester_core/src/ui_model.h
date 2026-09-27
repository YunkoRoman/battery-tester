#pragma once
#include <stdint.h>

#include "settings.h"

namespace tc {

enum class Screen : uint8_t { Overview, ChannelView, Settings, Service };
enum class Key : uint8_t { Left, Right, Ok, OkLong };
enum class SettingsItem : uint8_t { Current, Cutoff, Mode, StartStop, Back, Count };
enum class ServiceOut : uint8_t { Off, Relay, Load };

struct UiState {
    Screen screen;
    uint8_t ch;       // selected channel (Overview / ChannelView / Settings)
    uint8_t item;     // SettingsItem as uint8_t
    bool editing;
    ServiceOut svc[kChannels];
    bool svcFan;
    uint8_t svcRow;   // 0..3 channels, 4 = fan
};

enum class UiAction : uint8_t { None, Start, Stop, SettingsChanged, ExitService };

struct UiResult {
    UiAction action;
    uint8_t ch;
};

UiState initialUi(bool service);
UiResult handleKey(UiState& ui, Key key, ChannelSettings* settings, const bool* running);

}  // namespace tc
