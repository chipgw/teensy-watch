#include "timermode.hpp"
#include "watchcore.hpp"
#include <Time.h>

TimerMode::TimerMode(WatchCore& c) : WatchMode(c), setting(90), remaining(2), running(false), setTimer(false), setAmount(1) { }

void TimerMode::draw(Adafruit_GFX &display) {
    display.setTextSize(2);
    display.print("Timer");
    display.println();

    if (setTimer) {
        /* When setting the timer value, blink the currently selected part. */
        bool blinkOff = (millis() % 1000) < 500;

        display.setTextSize(3);
        if (!blinkOff || setAmount != SECS_PER_HOUR)
            display.printf("%02i", setting / SECS_PER_HOUR);
        else
            display.print("  ");

        if (!blinkOff || setAmount != SECS_PER_MIN)
            display.printf(":%02i", minute(setting));
        else
            display.print(":  ");

        display.setTextSize(2);
        if (!blinkOff || setAmount != 1)
            display.printf(":%02i", second(setting));
        else
            display.print(":  ");
    } else {
        display.setTextSize(3);
        display.printf("%02i:%02i", remaining / SECS_PER_HOUR, minute(remaining));
        display.setTextSize(2);
        display.printf(":%02i", second(remaining));
    }
}

namespace {

const WatchMenu menu[] = {
    { "Start/Stop", [](WatchMode* mode, WatchCore& core) {
          TimerMode* timer = static_cast<TimerMode*>(mode);

          if (timer)
              timer->running = !timer->running;

          return true;
      }},
    { "Reset", [](WatchMode* mode, WatchCore& core) {
          TimerMode* timer = static_cast<TimerMode*>(mode);

          if (timer) {
              timer->remaining = timer->setting;
              timer->running = false;
          }
          return true;
      }},
    { "Set Timer", [](WatchMode* mode, WatchCore& core) {
          TimerMode* timer = static_cast<TimerMode*>(mode);

          if (timer) {
              timer->running = false;
              timer->setTimer = true;
          }
          return true;
     }},
    lightMenu,
    modeMenu,
    { nullptr }
};

}

void TimerMode::buttonPress(time_t buttonTime) {
    if (setTimer) {
        setTimer = false;
        remaining = setting;
    } else {
        core.openMenu(menu);
    }
}

void TimerMode::left(uint8_t amount) {
    if (setTimer) {
        if (setAmount == SECS_PER_HOUR)
            setAmount = 1;
        else
            setAmount *= 60;
    }
}

void TimerMode::right(uint8_t amount) {
    if (setTimer) {
        if (setAmount == 1)
            setAmount = SECS_PER_HOUR;
        else
            setAmount /= 60;
    }
}

void TimerMode::up(uint8_t amount) {
    if (setTimer) {
        setting += amount * setAmount;

        /* If the timer is 100 hours or more loop back to 0 hours. */
        if (setting >= 100 * SECS_PER_HOUR)
            setting -= 100 * SECS_PER_HOUR;
    }
}

void TimerMode::down(uint8_t amount) {
    if (setTimer) {
        /* If there is less time in the setting than we'd be subtracting, loop around to the maximum amount for the amount. */
        if (amount * setAmount > setting) {
            if (setAmount == SECS_PER_HOUR)
                setting += 99 * SECS_PER_HOUR;
            else
                setting += 60 * setAmount;
        }
        setting -= amount * setAmount;
    }
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
