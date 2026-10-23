#include "display.h"

#include <NanitLib.h>

namespace {

const uint16_t COLOR_BG = ST7735_BLACK;
const uint16_t COLOR_LABEL = ST7735_CYAN;
const uint16_t COLOR_VALUE = ST7735_WHITE;

// Координати полів (екран 160x128, textSize 2 = 12x16 px на символ)
const int16_t X_LABEL = 4;
const int16_t X_VALUE = 40;
const int16_t Y_STATUS = 4;
const int16_t Y_VOLTAGE = 24;
const int16_t Y_CURRENT = 48;
const int16_t Y_CAPACITY = 72;
const int16_t Y_TIME = 100;
const int16_t Y_NANIT = 119;  // textSize 1 (8 px) — вміщається під часом

// Ширина поля значення в символах — старе значення перезаписується пробілами
const uint8_t VALUE_WIDTH = 7;

// Друк тексту фіксованої ширини з фоном, щоб затерти попереднє значення
void printField(int16_t x, int16_t y, uint8_t size, uint16_t color,
                const char *text, uint8_t width) {
    tft.setTextSize(size);
    tft.setTextColor(color, COLOR_BG);
    tft.setCursor(x, y);
    uint8_t len = strlen(text);
    tft.print(text);
    for (uint8_t i = len; i < width; i++) {
        tft.print(' ');
    }
}

void printLabel(int16_t y, const char *label) {
    tft.setTextSize(1);
    tft.setTextColor(COLOR_LABEL, COLOR_BG);
    tft.setCursor(X_LABEL, y + 4);
    tft.print(label);
}

void printFloat(int16_t y, float value, uint8_t decimals) {
    char buf[12];
    dtostrf(value, 0, decimals, buf);
    printField(X_VALUE, y, 2, COLOR_VALUE, buf, VALUE_WIDTH);
}

const char *stateText(TesterState state, uint16_t &color) {
    switch (state) {
        case TesterState::Discharging:
            color = ST7735_YELLOW;
            return "DISCHARGING";
        case TesterState::Done:
            color = ST7735_GREEN;
            return "DONE";
        case TesterState::Removed:
            color = ST7735_RED;
            return "BATT REMOVED";
        case TesterState::NoBattery:
            color = ST7735_RED;
            return "NO BATTERY";
        case TesterState::Idle:
        default:
            color = ST7735_WHITE;
            return "IDLE";
    }
}

}  // namespace

void displayInit() {
    tft.fillScreen(COLOR_BG);

    printLabel(Y_VOLTAGE, "V");
    printLabel(Y_CURRENT, "mA");
    printLabel(Y_CAPACITY, "mAh");
    printLabel(Y_TIME - 4, "Time");
    printLabel(Y_NANIT - 4, "Nanit");
}

void displayUpdate(TesterState state, float voltage, float currentMa,
                   float capacityMah, unsigned long elapsedSec, uint8_t nanitPercent) {
    uint16_t statusColor;
    const char *status = stateText(state, statusColor);
    printField(X_LABEL, Y_STATUS, 2, statusColor, status, 13);

    printFloat(Y_VOLTAGE, voltage, 2);
    printFloat(Y_CURRENT, currentMa, 0);
    printFloat(Y_CAPACITY, capacityMah, 0);

    char timeBuf[10];
    unsigned int h = elapsedSec / 3600;
    unsigned int m = (elapsedSec / 60) % 60;
    unsigned int s = elapsedSec % 60;
    snprintf(timeBuf, sizeof(timeBuf), "%02u:%02u:%02u", h, m, s);
    printField(X_VALUE, Y_TIME, 2, COLOR_VALUE, timeBuf, 8);

    char nanitBuf[6];
    snprintf(nanitBuf, sizeof(nanitBuf), "%u%%", nanitPercent);
    printField(X_VALUE, Y_NANIT, 1, COLOR_VALUE, nanitBuf, 4);
}
