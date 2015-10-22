#pragma once

#include <Adafruit_SSD1306.h>

#define BUTTON_ONE 8
#define BUTTON_TWO 6

#define BUZZER_PIN 4

#define SET_PRESS_TIME 2

class WatchCore {
    Adafruit_SSD1306 display;

    enum Mode {
        Time,
        TimeSet,
        Tempurature,
        Timer,
        Stopwatch
    };

    Mode mode;

    time_t buttonOneTime;
    time_t buttonTwoTime;

    void doInput();

public:
    WatchCore();

    void run();

    void drawTime();
    void drawTempurature();
    void drawTimer();

    void switchMode(time_t buttonTime);


    time_t timerStart;
    time_t timerLength;
};
