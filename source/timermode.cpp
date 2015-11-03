#include "timermode.hpp"
#include "watchcore.hpp"
#include <Adafruit_GFX.h>
#include <Time.h>

TimerMode::TimerMode(WatchCore& c) : WatchMode(c), setting(90), remaining(2), running(false) { }

void TimerMode::draw(Adafruit_GFX &display) {
    display.setTextSize(2);
    display.print("Timer");
    display.println();

    display.setTextSize(3);
    display.printf("%02i:%02i", hour(remaining), minute(remaining));
    display.setTextSize(2);
    display.printf(":%02i", second(remaining));
}

namespace {

WatchMenu menu[] = {
    { "Start/Stop", [](WatchMode* mode, WatchCore& core) {
          TimerMode* timer = static_cast<TimerMode*>(mode);

          if (timer)
              timer->running = !timer->running;

          return true;
      }, nullptr },
    { "Reset", [](WatchMode* mode, WatchCore& core) {
          TimerMode* timer = static_cast<TimerMode*>(mode);

          if (timer) {
              timer->remaining = timer->setting;
              timer->running = false;
          }
          return true;
      }, nullptr },
    { "Set Timer", [](WatchMode* mode, WatchCore& core) {
          /* TODO - Implement. */
          return true;
     }, nullptr },
    modeMenu,
    { nullptr, nullptr, nullptr }
};

}

void TimerMode::buttonPress(time_t buttonTime) {
    core.openMenu(menu);
}

void TimerMode::left(uint8_t amount) {
}

void TimerMode::right(uint8_t amount) {
}

void TimerMode::up(uint8_t amount) {
}

void TimerMode::down(uint8_t amount) {
}

void TimerMode::tick(time_t delta) {
    if (running) {
        if (remaining > delta)
            remaining -= delta;
        else {
            remaining = 0;
            core.enableBuzzer(60);
            running = false;
        }
    }
}
