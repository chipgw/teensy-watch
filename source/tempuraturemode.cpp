#include "tempuraturemode.hpp"
#include "watchcore.hpp"

TempuratureMode::TempuratureMode(WatchCore& c) : WatchMode(c), freeze(false) { }

void TempuratureMode::draw(Adafruit_GFX &display) {
    display.setTextColor(WHITE);

    display.setCursor(0, 4);
    display.setTextSize(3);
    display.print(temperatureC);
    display.setTextSize(1);
    display.print("O");
    display.setTextSize(2);
    display.print("C");

    display.setCursor(0, 36);
    display.setTextSize(3);
    display.print(temperatureF);
    display.setTextSize(1);
    display.print("O");
    display.setTextSize(2);
    display.print("F");

    display.setTextSize(1);
    display.setCursor(90, 8);
    display.printf("%imV", mV);

    if (freeze) {
        display.setCursor(90, 40);
        display.print("LOCKED");
    }
}


namespace {

WatchMenu menu[] = {
    { "Lock", [](WatchMode* mode, WatchCore& core) {
          TempuratureMode* temp = static_cast<TempuratureMode*>(mode);

          if (temp)
              temp->freeze = !temp->freeze;

          return true;
      }, nullptr },
    modeMenu,
    { nullptr, nullptr, nullptr }
};

}

void TempuratureMode::buttonPress(time_t buttonTime) {
    core.openMenu(menu);
}

void TempuratureMode::left(uint8_t amount) {
}

void TempuratureMode::right(uint8_t amount) {
}

void TempuratureMode::up(uint8_t amount) {
}

void TempuratureMode::down(uint8_t amount) {
}

void TempuratureMode::tick(time_t delta) {
    /* Only update the values once per second to make it easier to read. */
    if (delta && !freeze) {
        mV = analogRead(A8) * 3300 / 1024;

        /* Each MV is 0.1C, and a reading of 0 is 50C below zero. */
        temperatureC = mV / 10 - 50;

        /* Each MV is 0.18F (9 / 50), and a reading of 0 is 58F below zero. */
        temperatureF = mV * 9 / 50 - 58;
    }
}
