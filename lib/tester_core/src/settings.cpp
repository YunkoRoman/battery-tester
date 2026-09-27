#include "settings.h"

namespace tc {

namespace {

const uint8_t kMagic = 0xB7;
const uint8_t kVersion = 1;

uint16_t stepClamp(uint16_t value, int8_t dir, uint16_t step, uint16_t lo, uint16_t hi) {
    int32_t next = (int32_t)value + (int32_t)dir * (int32_t)step;
    if (next < lo) next = lo;
    if (next > hi) next = hi;
    return (uint16_t)next;
}

}  // namespace

uint8_t crc8(const uint8_t* data, uint8_t len) {
    uint8_t crc = 0;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; bit++) {
            crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x07) : (uint8_t)(crc << 1);
        }
    }
    return crc;
}

ChannelSettings defaultSettings() {
    ChannelSettings s;
    s.currentMa = 500;
    s.cutoffMv = 3000;
    s.mode = Mode::Full;
    return s;
}

void stepCurrent(ChannelSettings& s, int8_t dir) {
    s.currentMa = stepClamp(s.currentMa, dir, kCurrentStepMa, kCurrentMinMa, kCurrentMaxMa);
}

void stepCutoff(ChannelSettings& s, int8_t dir) {
    s.cutoffMv = stepClamp(s.cutoffMv, dir, kCutoffStepMv, kCutoffMinMv, kCutoffMaxMv);
}

void stepMode(ChannelSettings& s, int8_t dir) {
    int8_t m = (int8_t)s.mode + (dir > 0 ? 1 : -1);
    if (m < 0) m = 2;
    if (m > 2) m = 0;
    s.mode = (Mode)m;
}

void packSettings(const ChannelSettings* s, uint8_t* out) {
    out[0] = kMagic;
    out[1] = kVersion;
    uint8_t k = 2;
    for (uint8_t c = 0; c < kChannels; c++) {
        out[k++] = (uint8_t)(s[c].currentMa & 0xFF);
        out[k++] = (uint8_t)(s[c].currentMa >> 8);
        out[k++] = (uint8_t)(s[c].cutoffMv & 0xFF);
        out[k++] = (uint8_t)(s[c].cutoffMv >> 8);
        out[k++] = (uint8_t)s[c].mode;
    }
    out[k] = crc8(out, k);
}

bool unpackSettings(const uint8_t* in, ChannelSettings* s) {
    const uint8_t n = kSettingsBlobSize - 1;
    if (in[0] != kMagic || in[1] != kVersion || crc8(in, n) != in[n]) return false;
    ChannelSettings tmp[kChannels];
    uint8_t k = 2;
    for (uint8_t c = 0; c < kChannels; c++) {
        uint16_t current = (uint16_t)(in[k] | (in[k + 1] << 8));
        uint16_t cutoff = (uint16_t)(in[k + 2] | (in[k + 3] << 8));
        uint8_t mode = in[k + 4];
        k += 5;
        if (current < kCurrentMinMa || current > kCurrentMaxMa) return false;
        if (cutoff < kCutoffMinMv || cutoff > kCutoffMaxMv) return false;
        if (mode > 2) return false;
        tmp[c].currentMa = current;
        tmp[c].cutoffMv = cutoff;
        tmp[c].mode = (Mode)mode;
    }
    for (uint8_t c = 0; c < kChannels; c++) s[c] = tmp[c];
    return true;
}

}  // namespace tc
