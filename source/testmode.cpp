#include "testmode.hpp"
#include "watchcore.hpp"
#include <Adafruit_GFX.h>

TestMode::TestMode(WatchCore& c) : WatchMode(c), x(64), y(32) { }

void TestMode::draw(Adafruit_GFX& display) {
    display.drawPixel(x, y, WHITE);
}

namespace {

WatchMenu menu[] = {
//    { "", [](WatchMode* mode, WatchCore& core) {
//          return true;
//     }, nullptr },
    modeMenu,
    { nullptr, nullptr, nullptr }
};

}

void TestMode::buttonPress(time_t buttonTime) {
    core.openMenu(menu);
}

void TestMode::left(uint8_t amount) {
    x -= amount;
}

void TestMode::right(uint8_t amount) {
    x += amount;
}

void TestMode::up(uint8_t amount) {
    y -= amount;
}

void TestMode::down(uint8_t amount) {
    y += amount;
}

void TestMode::tick(time_t delta) {
    digitalWriteFast(TRACKBALL_WHT, x > 64);
    digitalWriteFast(TRACKBALL_RED, x < 64);
    digitalWriteFast(TRACKBALL_BLU, y > 32);
    digitalWriteFast(TRACKBALL_GRN, y < 32);
}
