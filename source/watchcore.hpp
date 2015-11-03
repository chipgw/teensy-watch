#pragma once

#include <Adafruit_SSD1306.h>

#define TRACKBALL_BTN 0
#define TRACKBALL_LFT 1
#define TRACKBALL_RGT 2
#define TRACKBALL_UP  3
#define TRACKBALL_DWN 4

#define TRACKBALL_WHT 5
#define TRACKBALL_GRN 6
#define TRACKBALL_RED 7
#define TRACKBALL_BLU 8

#define BUZZER_PIN 10

#define SET_PRESS_TIME 2

class WatchMenu;
class WatchMode;

class WatchCore {
    Adafruit_SSD1306 display;

    time_t last;

    time_t buttonTime;

    void doInput();

    WatchMode* modes[4];

    /* End time for the buzzer. */
    time_t buzzer;

    uint8_t lftLast;
    uint8_t rgtLast;
    uint8_t upLast;
    uint8_t dwnLast;

    const WatchMenu* currentMenu;
    int currentMenuItem;
    int currentMenuLength;

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

    void openMenu(const WatchMenu* menu);

    void run();

    void enableBuzzer(time_t seconds);
};
