#pragma once
#include <stdint.h>

#include "channel.h"
#include "settings.h"
#include "ui_model.h"

namespace tc {

const uint8_t kLineChars = 26;  // 160 px / 6 px per character at text size 1
const uint8_t kLineBuf = kLineChars + 1;

// scaled = value * 10^decimals, decimals 0..2. AVR printf has no %f.
void fmtFixed(char* out, uint8_t size, int32_t scaled, uint8_t decimals);
void fmtHms(char* out, uint8_t size, uint32_t seconds);

const char* stageShort(const Channel& ch);  // always 4 characters
const char* stageLong(const Channel& ch);

void headerLine(char* out, const char* title, uint8_t nanitPct);
void overviewRow(char* out, uint8_t idx, const Channel& ch);
void channelLine(char* out, uint8_t row, const Channel& ch);  // rows 0..6
void settingsLine(char* out, uint8_t row, const ChannelSettings& s, uint8_t selected, bool editing, bool running);
void serviceRow(char* out, uint8_t idx, const Measurement& m, ServiceOut o);
void serviceFooter(char* out, float busV, bool fan);

}  // namespace tc
