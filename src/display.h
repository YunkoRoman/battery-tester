#pragma once
#include <stdint.h>

#include "channel.h"
#include "settings.h"
#include "ui_model.h"

namespace view {

struct Frame {
    const tc::UiState* ui;
    const tc::Channel* ch;               // kChannels
    const tc::Measurement* meas;         // kChannels
    const tc::ChannelSettings* settings; // kChannels
    bool powerOk;
    float busV;
    uint8_t nanitPct;
};

void begin();
void render(const Frame& f);

}  // namespace view
