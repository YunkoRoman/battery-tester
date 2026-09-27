#include "display.h"

#include <NanitLib.h>
#include <stdio.h>

#include "view_text.h"

namespace view {

namespace {

const uint16_t kBg = ST7735_BLACK;
const uint16_t kGrey = 0x7BEF;
const uint16_t kSelBg = 0x10A2;
const uint8_t kRowPx = 12;
uint8_t lastScreen = 0xFF;

void line(uint8_t row, const char* text, uint16_t fg, uint16_t bg = kBg) {
    char buf[tc::kLineBuf];
    snprintf(buf, sizeof buf, "%-26s", text);  // pad so the previous text is overwritten
    tft.setTextColor(fg, bg);
    tft.setCursor(2, 4 + row * kRowPx);
    tft.print(buf);
}

uint16_t stageColor(const tc::Channel& ch) {
    if (ch.paused()) return ST7735_MAGENTA;
    switch (ch.stage()) {
        case tc::Stage::Fault: return ST7735_RED;
        case tc::Stage::Done: return ST7735_GREEN;
        case tc::Stage::RiTest:
        case tc::Stage::Discharge: return ST7735_YELLOW;
        case tc::Stage::Charge:
        case tc::Stage::Recharge: return ST7735_CYAN;
        case tc::Stage::Empty: return kGrey;
        default: return ST7735_WHITE;
    }
}

void footer(bool powerOk, const char* hint) {
    if (!powerOk) line(9, "NO POWER: 5V BUS OFF", ST7735_MAGENTA);
    else line(9, hint, kGrey);
}

}  // namespace

void begin() {
    tft.setRotation(1);
    tft.setTextWrap(false);
    tft.setTextSize(1);
    tft.fillScreen(kBg);
}

void render(const Frame& f) {
    if ((uint8_t)f.ui->screen != lastScreen) {
        tft.fillScreen(kBg);
        lastScreen = (uint8_t)f.ui->screen;
    }
    char buf[tc::kLineBuf];
    char title[tc::kLineBuf];
    const uint8_t sel = f.ui->ch;

    switch (f.ui->screen) {
        case tc::Screen::Overview:
            tc::headerLine(buf, "TESTER 4x18650", f.nanitPct);
            line(0, buf, ST7735_CYAN);
            for (uint8_t c = 0; c < tc::kChannels; c++) {
                tc::overviewRow(buf, c, f.ch[c]);
                line(2 + c * 2, buf, stageColor(f.ch[c]), c == sel ? kSelBg : kBg);
            }
            footer(f.powerOk, "<> select  OK open");
            break;

        case tc::Screen::ChannelView:
            snprintf(title, sizeof title, "CH%u %s", (unsigned)(sel + 1), tc::stageLong(f.ch[sel]));
            tc::headerLine(buf, title, f.nanitPct);
            line(0, buf, stageColor(f.ch[sel]));
            for (uint8_t r = 0; r < 7; r++) {
                tc::channelLine(buf, r, f.ch[sel]);
                line(2 + r, buf, ST7735_WHITE);
            }
            footer(f.powerOk, "<> chan OK menu hold=back");
            break;

        case tc::Screen::Settings:
            snprintf(title, sizeof title, "CH%u SETTINGS", (unsigned)(sel + 1));
            tc::headerLine(buf, title, f.nanitPct);
            line(0, buf, ST7735_CYAN);
            for (uint8_t r = 0; r < (uint8_t)tc::SettingsItem::Count; r++) {
                bool selected = r == f.ui->item;
                tc::settingsLine(buf, r, f.settings[sel], f.ui->item, f.ui->editing, f.ch[sel].running());
                line(2 + r, buf, selected && f.ui->editing ? ST7735_YELLOW : ST7735_WHITE, selected ? kSelBg : kBg);
            }
            footer(f.powerOk, "<> move OK edit hold=back");
            break;

        case tc::Screen::Service:
            line(0, "SERVICE MODE", ST7735_YELLOW);
            for (uint8_t c = 0; c < tc::kChannels; c++) {
                tc::serviceRow(buf, c, f.meas[c], f.ui->svc[c]);
                line(2 + c, buf, ST7735_WHITE, f.ui->svcRow == c ? kSelBg : kBg);
            }
            tc::serviceFooter(buf, f.busV, f.ui->svcFan);
            line(7, buf, ST7735_WHITE, f.ui->svcRow == tc::kChannels ? kSelBg : kBg);
            line(9, "<> row OK set hold=exit", kGrey);
            break;
    }
}

}  // namespace view
