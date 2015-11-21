#pragma once

#include <Adafruit_SSD1351.h>

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define TRACKBALL_BTN 0
#define TRACKBALL_LFT 1
#define TRACKBALL_RGT 2
#define TRACKBALL_UP  3
#define TRACKBALL_DWN 4

#define TRACKBALL_WHT 5
#define TRACKBALL_GRN 6
#define TRACKBALL_RED 7
#define TRACKBALL_BLU 8

#define BUZZER_PIN 18

#define LIGHT_PIN 17

class WatchMenu;
class WatchMode;

class WatchCore {
    Adafruit_SSD1351 display;

    time_t last;

    time_t buttonTime;

    void doInput();

    WatchMode* modes[5];

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
        Stopwatch,
        Test
    };

    Mode currentMode;

    WatchCore();
    ~WatchCore();

    void openMenu(const WatchMenu* menu);
    void popMenu();

    void run();

    void enableBuzzer(time_t seconds);

    void setCurrentTime(time_t time);
};
