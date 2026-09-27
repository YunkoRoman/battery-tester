#include "view_text.h"

#include <stdio.h>
#include <string.h>

namespace tc {

namespace {

int32_t scaled(float x, int32_t factor) {
    return (int32_t)(x * factor + (x >= 0.0f ? 0.5f : -0.5f));
}

const char* modeName(Mode m) {
    switch (m) {
        case Mode::Discharge: return "DISCHARGE";
        case Mode::Charge: return "CHARGE";
        default: return "FULL";
    }
}

}  // namespace

void fmtFixed(char* out, uint8_t size, int32_t v, uint8_t decimals) {
    const char* sign = "";
    if (v < 0) {
        sign = "-";
        v = -v;
    }
    switch (decimals) {
        case 1: snprintf(out, size, "%s%ld.%01ld", sign, (long)(v / 10), (long)(v % 10)); break;
        case 2: snprintf(out, size, "%s%ld.%02ld", sign, (long)(v / 100), (long)(v % 100)); break;
        default: snprintf(out, size, "%s%ld", sign, (long)v); break;
    }
}

void fmtHms(char* out, uint8_t size, uint32_t s) {
    unsigned long h = s / 3600UL;
    if (h > 99) h = 99;
    snprintf(out, size, "%02lu:%02lu:%02lu", h, (unsigned long)((s / 60UL) % 60UL), (unsigned long)(s % 60UL));
}

const char* stageShort(const Channel& ch) {
    if (ch.paused()) return "PAUS";
    switch (ch.stage()) {
        case Stage::Empty: return "--- ";
        case Stage::Idle: return "IDLE";
        case Stage::Charge: return "CHG ";
        case Stage::RestCharged:
        case Stage::RestDischarged: return "REST";
        case Stage::RiTest: return "RI  ";
        case Stage::Discharge: return "DIS ";
        case Stage::Recharge: return "RCHG";
        case Stage::Done: return "DONE";
        case Stage::Fault: break;
    }
    switch (ch.fault()) {
        case Fault::Dead: return "DEAD";
        case Fault::BadRi: return "BAD ";
        case Fault::Sag: return "SAG ";
        case Fault::Hot: return "HOT ";
        case Fault::ChargeTimeout: return "TOUT";
        case Fault::Removed: return "REM ";
        case Fault::SensorError: return "NTC?";
        case Fault::NoCurrent: return "NOI ";
        default: return "ERR ";
    }
}

const char* stageLong(const Channel& ch) {
    if (ch.paused()) return "NO POWER";
    switch (ch.stage()) {
        case Stage::Empty: return "NO CELL";
        case Stage::Idle: return "IDLE";
        case Stage::Charge: return "CHARGE";
        case Stage::RestCharged:
        case Stage::RestDischarged: return "REST";
        case Stage::RiTest: return "RI TEST";
        case Stage::Discharge: return "DISCHARGE";
        case Stage::Recharge: return "RECHARGE";
        case Stage::Done: return "DONE";
        case Stage::Fault: break;
    }
    switch (ch.fault()) {
        case Fault::Dead: return "DEAD CELL";
        case Fault::BadRi: return "BAD RI";
        case Fault::Sag: return "SAG";
        case Fault::Hot: return "HOT";
        case Fault::ChargeTimeout: return "CHG TIMEOUT";
        case Fault::Removed: return "REMOVED";
        case Fault::SensorError: return "NTC ERROR";
        case Fault::NoCurrent: return "NO CURRENT";
        default: return "FAULT";
    }
}

void headerLine(char* out, const char* title, uint8_t nanitPct) {
    char right[12];
    snprintf(right, sizeof right, "NANIT %u%%", (unsigned)nanitPct);
    size_t tl = strlen(title);
    size_t rl = strlen(right);
    if (tl + rl + 1 > kLineChars) {  // too long: one space, truncated to the line
        snprintf(out, kLineBuf, "%s %s", title, right);
        return;
    }
    // Manual padding: avr-libc printf support for "%*s" is not relied upon.
    memcpy(out, title, tl);
    memset(out + tl, ' ', kLineChars - tl - rl);
    memcpy(out + kLineChars - rl, right, rl);
    out[kLineChars] = '\0';
}

void overviewRow(char* out, uint8_t idx, const Channel& ch) {
    if (ch.stage() == Stage::Empty) {
        snprintf(out, kLineBuf, "%u ---  no cell", (unsigned)(idx + 1));
        return;
    }
    char v[8], a[8];
    fmtFixed(v, sizeof v, scaled(ch.lastCellV(), 100), 2);
    fmtFixed(a, sizeof a, scaled(ch.lastCurrentA(), 100), 2);
    snprintf(out, kLineBuf, "%u %s %sV %sA %ldmAh", (unsigned)(idx + 1), stageShort(ch), v, a,
             (long)scaled(ch.results().mAh, 1));
}

void channelLine(char* out, uint8_t row, const Channel& ch) {
    char n[12];
    char t[10];
    const Results& r = ch.results();
    switch (row) {
        case 0:
            fmtFixed(n, sizeof n, scaled(ch.lastCellV(), 100), 2);
            snprintf(out, kLineBuf, "V     %s V", n);
            break;
        case 1:
            fmtFixed(n, sizeof n, scaled(ch.lastCurrentA(), 100), 2);
            snprintf(out, kLineBuf, "I     %s A", n);
            break;
        case 2:
            if (ch.lastTempC() == kTempInvalid) snprintf(out, kLineBuf, "T     --");
            else if (r.maxTempC > 0.0f)
                snprintf(out, kLineBuf, "T     %ld C  max %ld", (long)scaled(ch.lastTempC(), 1), (long)scaled(r.maxTempC, 1));
            else snprintf(out, kLineBuf, "T     %ld C", (long)scaled(ch.lastTempC(), 1));
            break;
        case 3:
            fmtFixed(n, sizeof n, scaled(r.mWh / 1000.0f, 100), 2);
            snprintf(out, kLineBuf, "CAP   %ld mAh %sWh", (long)scaled(r.mAh, 1), n);
            break;
        case 4:
            if (r.riMilliOhm == 0) snprintf(out, kLineBuf, "Ri    --");
            else snprintf(out, kLineBuf, "Ri    %u mOhm", (unsigned)r.riMilliOhm);
            break;
        case 5:
            fmtHms(t, sizeof t, r.dischargeSeconds);
            snprintf(out, kLineBuf, "DIS   %s", t);
            break;
        default:
            fmtHms(t, sizeof t, ch.stageSeconds());
            snprintf(out, kLineBuf, "STAGE %s", t);
            break;
    }
}

void settingsLine(char* out, uint8_t row, const ChannelSettings& s, uint8_t selected, bool editing, bool running) {
    char mark = row == selected ? (editing ? '*' : '>') : ' ';
    char n[10];
    switch (row) {
        case 0:
            fmtFixed(n, sizeof n, s.currentMa / 10, 2);
            snprintf(out, kLineBuf, "%c CURRENT   %s A", mark, n);
            break;
        case 1:
            fmtFixed(n, sizeof n, s.cutoffMv / 10, 2);
            snprintf(out, kLineBuf, "%c CUTOFF    %s V", mark, n);
            break;
        case 2:
            snprintf(out, kLineBuf, "%c MODE      %s", mark, modeName(s.mode));
            break;
        case 3:
            snprintf(out, kLineBuf, "%c %s", mark, running ? "STOP" : "START");
            break;
        default:
            snprintf(out, kLineBuf, "%c BACK", mark);
            break;
    }
}

void serviceRow(char* out, uint8_t idx, const Measurement& m, ServiceOut o) {
    char v[8], a[8], t[6];
    fmtFixed(v, sizeof v, scaled(m.cellV, 100), 2);
    fmtFixed(a, sizeof a, scaled(m.currentA, 100), 2);
    if (m.tempC == kTempInvalid) snprintf(t, sizeof t, "--");
    else snprintf(t, sizeof t, "%ld", (long)scaled(m.tempC, 1));
    const char* out_name = o == ServiceOut::Relay ? "RELAY" : (o == ServiceOut::Load ? "LOAD" : "OFF");
    snprintf(out, kLineBuf, "%u %sV %sA %sC %c %s", (unsigned)(idx + 1), v, a, t, m.chargeDone ? 'D' : '-', out_name);
}

void serviceFooter(char* out, float busV, bool fan) {
    char v[8];
    fmtFixed(v, sizeof v, scaled(busV, 100), 2);
    snprintf(out, kLineBuf, "BUS %sV  FAN %s", v, fan ? "ON" : "OFF");
}

}  // namespace tc
