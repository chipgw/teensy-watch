#include "stopwatchmode.hpp"
#include "watchcore.hpp"

StopwatchMode::StopwatchMode(WatchCore& c) : WatchMode(c), running(false), length(0) { }

void StopwatchMode::draw(Adafruit_GFX &display) {
    display.setTextSize(2);
    display.print("Stopwatch");
    display.println();

    display.setTextSize(3);
    if (length >= 3600000) {
        display.printf("%02i:%02i", length / 3600000, length / 60000 % 60);
        display.setTextSize(2);
        display.printf(":%02i", length / 1000 % 60);
    } else {
        display.printf("%02i:%02i", length / 60000, length / 1000 % 60);
        display.setTextSize(2);
        display.printf(".%02i", length / 10 % 100);
    }
}


namespace {

WatchMenu menu[] = {
    { "Start/Stop", [](WatchMode* mode, WatchCore& core) {
          StopwatchMode* sw = static_cast<StopwatchMode*>(mode);

          if (sw)
              sw->running = !sw->running;

          return true;
      }, nullptr },
    { "Reset", [](WatchMode* mode, WatchCore& core) {
          StopwatchMode* sw = static_cast<StopwatchMode*>(mode);

          if (sw) {
              sw->length = 0;
              sw->running = false;
          }
          return true;
      }, nullptr },
    modeMenu,
    { nullptr, nullptr, nullptr }
};

}

void StopwatchMode::buttonPress(time_t buttonTime) {
    core.openMenu(menu);
}

void StopwatchMode::left(uint8_t amount) {
}

void StopwatchMode::right(uint8_t amount) {
}

void StopwatchMode::up(uint8_t amount) {
}

void StopwatchMode::down(uint8_t amount) {
}

void StopwatchMode::tick(time_t delta) {
    if (running)
        length += millis() - last;

    last = millis();
}
