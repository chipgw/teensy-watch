#include "clockmode.hpp"
#include "watchcore.hpp"
#include <Time.h>
#include <Adafruit_GFX.h>

ClockMode::ClockMode(WatchCore& c) : WatchMode(c) { }

void ClockMode::draw(Adafruit_GFX& display) {
    /* TODO - Use time zones and show date. */
    display.setTextSize(3);
    display.printf("%02i:%02i", hour(), minute());
    display.setTextSize(2);
    display.printf(":%02i", second());
    display.println();
}

void ClockMode::buttonOnePress(time_t buttonTime) {
    core.currentMode = WatchCore::Tempurature;
}

void ClockMode::buttonTwoPress(time_t buttonTime) {

}

bool ClockMode::isBuzzer() {
    /* The clock never enables the buzzer. */
    return false;
}

void ClockMode::tick(time_t delta) {

}
