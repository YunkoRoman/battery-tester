#pragma once
#include <stdint.h>

namespace tc {

const uint8_t kChannels = 4;

enum class Mode : uint8_t { Full = 0, Discharge = 1, Charge = 2 };

struct ChannelSettings {
    uint16_t currentMa;
    uint16_t cutoffMv;
    Mode mode;
};

const uint16_t kCurrentMinMa = 100;
const uint16_t kCurrentMaxMa = 1000;
const uint16_t kCurrentStepMa = 50;
const uint16_t kCutoffMinMv = 2800;
const uint16_t kCutoffMaxMv = 3300;
const uint16_t kCutoffStepMv = 50;

ChannelSettings defaultSettings();
void stepCurrent(ChannelSettings& s, int8_t dir);
void stepCutoff(ChannelSettings& s, int8_t dir);
void stepMode(ChannelSettings& s, int8_t dir);

// EEPROM layout: magic, version, 4 × {current lo, hi, cutoff lo, hi, mode}, crc8.
const uint8_t kSettingsBlobSize = 2 + 5 * kChannels + 1;

uint8_t crc8(const uint8_t* data, uint8_t len);
void packSettings(const ChannelSettings* s, uint8_t* out);
// Returns false and leaves `s` untouched if the blob is blank, corrupt or out of range.
bool unpackSettings(const uint8_t* in, ChannelSettings* s);

}  // namespace tc
