#include "WProgram.h"
#include "watchcore.hpp"
#include "watchmode.hpp"
#include "clockmode.hpp"
#include "tempuraturemode.hpp"
#include "timermode.hpp"
#include "stopwatchmode.hpp"
#include "testmode.hpp"
#include <Wire.h>
#include <Time.h>

#define dc   16
#define cs   10
#define rst  9

WatchCore::WatchCore() : display(cs, dc, rst), buttonTime(0), currentMenu(nullptr), currentMenuItem(0), currentMode(Time) {
    /* Tell the Time library to get time from Teensy's RTC. */
    setSyncProvider([]() { return time_t(Teensy3Clock.get()); });

    modes[Time]         = new ClockMode(*this);
    modes[Tempurature]  = new TempuratureMode(*this);
    modes[Timer]        = new TimerMode(*this);
    modes[Stopwatch]    = new StopwatchMode(*this);
    modes[Test]         = new TestMode(*this);

    Serial.begin(9600);

    Wire.begin();

    Serial.println("init");
    Serial.flush();

    display.begin();

    /* Set up the trackball inputs. */
    pinMode(TRACKBALL_BTN, INPUT);
    pinMode(TRACKBALL_LFT, INPUT);
    pinMode(TRACKBALL_RGT, INPUT);
    pinMode(TRACKBALL_UP,  INPUT);
    pinMode(TRACKBALL_DWN, INPUT);

    /* And the trackball LEDs. */
    pinMode(TRACKBALL_WHT, OUTPUT);
    pinMode(TRACKBALL_GRN, OUTPUT);
    pinMode(TRACKBALL_RED, OUTPUT);
    pinMode(TRACKBALL_BLU, OUTPUT);

    /* The buzzer needs to be set up too. */
    pinMode(BUZZER_PIN, OUTPUT);

    /* The flashlight LED needs to be set up too. */
    pinMode(LIGHT_PIN, OUTPUT);

    /* Debug info on whether or not the RTC works. */
    if (timeStatus() != timeSet)
        Serial.println("Unable to sync with the RTC");
    else
        Serial.println("RTC has set the system time");

    /* Get the initial values of the trackball hall effect sensors. */
    lftLast = digitalRead(TRACKBALL_LFT);
    rgtLast = digitalRead(TRACKBALL_RGT);
    upLast  = digitalRead(TRACKBALL_UP);
    dwnLast = digitalRead(TRACKBALL_DWN);
}

WatchCore::~WatchCore() {
    for (WatchMode* mode : modes)
        delete mode;
}

void WatchCore::run() {
    Serial.println("run");

    while (true) {
        /* This should be 0 most of the time and 1 every now and then when the clock advances a second. */
        time_t delta = now() - last;
        last = now();

        doInput();

        for (WatchMode* mode : modes)
            mode->tick(delta);

        /* Only start the tone on ticks where the delta advances. */
        if (buzzer >= now() && delta > 0)
            tone(BUZZER_PIN, 4000, 500);

        /* Some default settings for the display. */
        display.clearScreen();
        display.setCursor(0,0);
        display.setTextColor(YELLOW);

        /* The mode handles the drawing itself. */
        modes[currentMode]->draw(display);

        if (currentMenu != nullptr) {
            /* Find the maximum length of menu item names. */
            size_t maxWidth = 0;

            for (int i = 0; i < currentMenuLength; ++i) {
                auto len = strlen(currentMenu[i].name);

                if (len > maxWidth)
                    maxWidth = len;
            }

            /* A character is 6 pixels wide. */
            maxWidth *= 6;

            /* Black out the area behind the menu. */
            display.fillRect(0, 0, maxWidth + 5, display.height(), BLACK);

            /* Draw a line on the right side of the menu. */
            display.drawLine(maxWidth + 6, 0, maxWidth + 6, display.height(), WHITE);

            display.setTextSize(1);
            display.setCursor(0, 0);

            int16_t overflow = (currentMenuLength * 8) - display.height() + 4;

            if (overflow > 0) {
                int16_t scroll = overflow * currentMenuItem / currentMenuLength;
                display.drawLine(maxWidth + 4, scroll, maxWidth + 4, display.height() - overflow + scroll, WHITE);

                display.setCursor(0, -scroll);
            }

            for (int i = 0; i < currentMenuLength; ++i) {
                if (i == currentMenuItem)
                    display.setTextColor(BLACK, WHITE);
                else
                    display.setTextColor(WHITE, BLACK);

                display.println(currentMenu[i].name);
            }
        }
        display.update();
    }
}

void WatchCore::setCurrentTime(time_t time) {
    /* Set the Teensy's RTC. */
    Teensy3Clock.set(time);

    /* Update the Time library. */
    setTime(time);

    /* Keep the delta calculation accurate. */
    last = time;
}

void WatchCore::doInput() {
    while (Serial.available()) {
        String data = Serial.readStringUntil(':');

        /* Try getting the time from serial. */
        if (data.endsWith("Time")) {
            const time_t DEFAULT_TIME = 1357041600;

            time_t t = Serial.parseInt();

            if (t > DEFAULT_TIME)
                setCurrentTime(t);
        }
    }

    uint32_t lft = 0;
    uint32_t rgt = 0;
    uint32_t up  = 0;
    uint32_t dwn = 0;

    uint8_t lftNew = 0;
    uint8_t rgtNew = 0;
    uint8_t upNew  = 0;
    uint8_t dwnNew = 0;

    /* Poll the hall effect sensors for any changes over 8 milliseconds. */
    for(uint32_t time = millis() + 4; time > millis();) {
        lftNew = digitalReadFast(TRACKBALL_LFT);
        rgtNew = digitalReadFast(TRACKBALL_RGT);
        upNew  = digitalReadFast(TRACKBALL_UP);
        dwnNew = digitalReadFast(TRACKBALL_DWN);

        lft += lftLast != lftNew;
        rgt += rgtLast != rgtNew;
        up  += upLast  != upNew;
        dwn += dwnLast != dwnNew;

        lftLast = lftNew;
        rgtLast = rgtNew;
        upLast  = upNew;
        dwnLast = dwnNew;
    }

    /* LOW is pressed. */
    if (digitalRead(TRACKBALL_BTN) == LOW) {
        if (buttonTime == 0)
            buttonTime = now();
    } else {
        if(buttonTime != 0) {
            /* If the buzzer is enabled all pressing a button does is stop it. */
            if (buzzer > now()) {
                buzzer = 0;
            } else if (currentMenu != nullptr) {
                if (currentMenu[currentMenuItem].callback != nullptr && currentMenu[currentMenuItem].callback(modes[currentMode], *this))
                    currentMenu = nullptr;
                else if (currentMenu[currentMenuItem].subMenu != nullptr)
                    openMenu(currentMenu[currentMenuItem].subMenu);
            } else {
                modes[currentMode]->buttonPress(now() - buttonTime);
            }
        }

        buttonTime = 0;
    }

    if (currentMenu != nullptr) {
        currentMenuItem += dwn - up;

        if (currentMenuItem < 0)
            currentMenuItem = currentMenuLength - 1;
        else if (currentMenuItem >= currentMenuLength)
            currentMenuItem = 0;

        if (lft > 0)
            popMenu();
        else if (rgt > 0 && currentMenu[currentMenuItem].subMenu != nullptr)
            openMenu(currentMenu[currentMenuItem].subMenu);
    } else {
        if (lft > 0)
            modes[currentMode]->left(lft);
        if (rgt > 0)
            modes[currentMode]->right(rgt);
        if (up > 0)
            modes[currentMode]->up(up);
        if (dwn > 0)
            modes[currentMode]->down(dwn);
    }
}

void WatchCore::openMenu(const WatchMenu* menu) {
    menu->previousMenu = currentMenu;
    currentMenu = menu;
    currentMenuItem = 0;
    currentMenuLength = 0;

    /* Count the number of items in the menu. */
    for (const WatchMenu* c = currentMenu; c->name != nullptr; ++c)
        currentMenuLength++;
}

void WatchCore::popMenu() {
    const WatchMenu* menu = currentMenu;
    currentMenu = menu->previousMenu;
    menu->previousMenu = nullptr;

    currentMenuItem = 0;
    currentMenuLength = 0;

    /* Count the number of items in the menu. */
    for (const WatchMenu* c = currentMenu; c->name != nullptr; ++c)
        currentMenuLength++;
}

void WatchCore::enableBuzzer(time_t seconds) {
    buzzer = now() + seconds;
}

namespace {

const WatchMenu menu[] = {
    { "Clock", [](WatchMode* mode, WatchCore& core) {
          core.currentMode = WatchCore::Time;
          return true;
     }},
    { "Thermometer", [](WatchMode* mode, WatchCore& core) {
          core.currentMode = WatchCore::Tempurature;
          return true;
      }},
    { "Timer", [](WatchMode* mode, WatchCore& core) {
          core.currentMode = WatchCore::Timer;
          return true;
      }},
    { "Stopwatch", [](WatchMode* mode, WatchCore& core) {
          core.currentMode = WatchCore::Stopwatch;
          return true;
      }},
    { "Test", [](WatchMode* mode, WatchCore& core) {
          core.currentMode = WatchCore::Test;
          return true;
      }},
    { nullptr }
};

/* Has to be static so that lightMode can be copied into menus. */
static bool toggleLight(WatchMode* mode, WatchCore& core) {
    static bool on = false;

    digitalWrite(LIGHT_PIN, on = !on);

    return false;
}
}

/* This menu item is to be used in every mode's menu. */
const WatchMenu modeMenu = { "Mode", nullptr, menu };

const WatchMenu lightMenu = { "Light", toggleLight };
