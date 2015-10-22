#include "tempuraturemode.hpp"
#include "watchcore.hpp"
#include <Adafruit_GFX.h>

TempuratureMode::TempuratureMode(WatchCore& c) : WatchMode(c) { }

void TempuratureMode::draw(Adafruit_GFX &display) {
    /* TODO - Something got messed up with the thermometer when switching from the 5v UNO to the 3.3v Teensy... */
    display.setTextSize(2);
    display.setTextColor(WHITE);

    int mV = analogRead(A2) * 3300 / 1024;

    display.print(mV); display.println("mV");

    int temperatureC = mV / 10 - 50;
    display.print(temperatureC);
    display.setTextSize(1);
    display.print("O");
    display.setTextSize(2);
    display.println("C");

    int temperatureF = (temperatureC * 9 / 5) + 32;
    display.print(temperatureF);
    display.setTextSize(1);
    display.print("O");
    display.setTextSize(2);
    display.println("F");
}

void TempuratureMode::buttonOnePress(time_t buttonTime) {
    core.currentMode = WatchCore::Timer;
}

void TempuratureMode::buttonTwoPress(time_t buttonTime) {

}

bool TempuratureMode::isBuzzer() {
    /* Thermometer does not buzz. */
    return false;
}

void TempuratureMode::tick(time_t delta) {

}
