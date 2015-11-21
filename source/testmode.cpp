#include "testmode.hpp"
#include "watchcore.hpp"

TestMode::TestMode(WatchCore& c) : WatchMode(c), x(64), y(48) { }

void TestMode::draw(Adafruit_GFX& display) {
    static uint32_t d;
    display.drawPixel(x, y, GREEN);
    display.setCursor(0, 88);
    display.setTextSize(1);
    display.print(millis() - d);
    d = millis();
}

namespace {

WatchMenu menu[] = {
//    { "", [](WatchMode* mode, WatchCore& core) {
//          return true;
//     }, nullptr },
    { "Test Buzzer 5 sec", [](WatchMode* mode, WatchCore& core) {
          core.enableBuzzer(5);
          return true;
      }, nullptr },
    { "Test Buzzer 20 sec", [](WatchMode* mode, WatchCore& core) {
          core.enableBuzzer(20);
          return true;
      }, nullptr },
    modeMenu,
    /* Lots of mode menus so we can test scrolling. */
    modeMenu,
    modeMenu,
    modeMenu,
    modeMenu,
    modeMenu,
    modeMenu,
    modeMenu,
    modeMenu,
    modeMenu,
    modeMenu,
    modeMenu,
    modeMenu,
    modeMenu,
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
    digitalWriteFast(TRACKBALL_BLU, y > 48);
    digitalWriteFast(TRACKBALL_GRN, y < 48);
}
