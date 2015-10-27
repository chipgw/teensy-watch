#pragma once

#include <Adafruit_SSD1306.h>

#define BUTTON_ONE 8
#define BUTTON_TWO 6

#define BUZZER_PIN 4

#define SET_PRESS_TIME 2

class WatchMode;

class WatchCore {
    Adafruit_SSD1306 display;

    time_t last;

    time_t buttonOneTime;
    time_t buttonTwoTime;

    void doInput();

    WatchMode* modes[4];

    /* End time for the buzzer. */
    time_t buzzer;

public:
    enum Mode {
        Time,
        Tempurature,
        Timer,
        Stopwatch
    };

    Mode currentMode;

    WatchCore();
    ~WatchCore();

    void run();

    void enableBuzzer(time_t seconds);
};
