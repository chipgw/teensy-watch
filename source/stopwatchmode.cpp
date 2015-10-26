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
        display.printf(":%02i", length / 10 % 100);
    }
    display.println();
}

void StopwatchMode::buttonOnePress(time_t buttonTime) {
    core.currentMode = WatchCore::Time;
}

void StopwatchMode::buttonTwoPress(time_t buttonTime) {
    if (!running && buttonTime > SET_PRESS_TIME)
        length = 0;
    else
        running = !running;
}

bool StopwatchMode::isBuzzer() {
    /* Stopwatch does not buzz. */
    return false;
}

void StopwatchMode::tick(time_t delta) {
    if (running)
        length += millis() - last;

    last = millis();
}
