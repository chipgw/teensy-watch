#include "timermode.hpp"
#include "watchcore.hpp"
#include <Adafruit_GFX.h>
#include <Time.h>

TimerMode::TimerMode(WatchCore& c) : WatchMode(c), setting(90), remaining(2), running(false) { }

void TimerMode::draw(Adafruit_GFX &display) {
    display.setTextSize(3);
    display.printf("%02i:%02i", hour(remaining), minute(remaining));
    display.setTextSize(2);
    display.printf(":%02i", second(remaining));
    display.println();
}

void TimerMode::buttonOnePress(time_t buttonTime) {
    core.currentMode = WatchCore::Stopwatch;
}

void TimerMode::buttonTwoPress(time_t buttonTime) {
    if (!running && (remaining == 0 || buttonTime > SET_PRESS_TIME))
        remaining = setting;
    else
        running = !running;
}

bool TimerMode::isBuzzer() {
    return running && remaining == 0;
}

void TimerMode::tick(time_t delta) {
    if (running) {
        if (remaining > delta)
            remaining -= delta;
        else
            remaining = 0;
    }
}
