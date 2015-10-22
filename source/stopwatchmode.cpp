#include "stopwatchmode.hpp"
#include "watchcore.hpp"

StopwatchMode::StopwatchMode(WatchCore& c) : WatchMode(c) { }

void StopwatchMode::draw(Adafruit_GFX &display) {

}

void StopwatchMode::buttonOnePress(time_t buttonTime) {
    core.currentMode = WatchCore::Time;
}

void StopwatchMode::buttonTwoPress(time_t buttonTime) {

}

bool StopwatchMode::isBuzzer() {
    /* Stopwatch does not buzz. */
    return false;
}

void StopwatchMode::tick(time_t delta) {

}
