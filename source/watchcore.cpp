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

time_t getTeensy3Time() {
  return Teensy3Clock.get();
}

WatchCore::WatchCore() : display(2), buttonTime(0), currentMenu(nullptr), currentMenuItem(0), currentMode(Time) {
    setSyncProvider(getTeensy3Time);
    pinMode(13, OUTPUT);

    digitalWriteFast(13, HIGH);
    delay(100);

    modes[Time]         = new ClockMode(*this);
    modes[Tempurature]  = new TempuratureMode(*this);
    modes[Timer]        = new TimerMode(*this);
    modes[Stopwatch]    = new StopwatchMode(*this);
    modes[Test]         = new TestMode(*this);

    Serial.begin(9600);

    Wire.begin();

    Serial.println("init1");
    Serial.flush();
    digitalWriteFast(13, LOW); delay(100);
    digitalWriteFast(13, HIGH); delay(100);

    int successes = 0;
    for (int address = 0x01; address < 0xff; ++address) {
        Wire.beginTransmission(address);
        int error = Wire.endTransmission();
        if(error == 0) {
            Serial.printf("Found device at 0x%02X\n", address);
            successes++;
        } else if(error == 4) {
            Serial.printf("Device error at 0x%02X\n", address);
        }
    }

    if (successes == 0)
        Serial.println("no devices found!");

    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

    /* In case anything is still being shown on the display from last program run. */
    display.clearDisplay();
    display.display();

    Serial.println("init2");
    Serial.flush();
    digitalWriteFast(LED_BUILTIN, LOW);

    pinMode(TRACKBALL_BTN, INPUT);
    pinMode(TRACKBALL_LFT, INPUT);
    pinMode(TRACKBALL_RGT, INPUT);
    pinMode(TRACKBALL_UP,  INPUT);
    pinMode(TRACKBALL_DWN, INPUT);

    pinMode(TRACKBALL_WHT, OUTPUT);
    pinMode(TRACKBALL_GRN, OUTPUT);
    pinMode(TRACKBALL_RED, OUTPUT);
    pinMode(TRACKBALL_BLU, OUTPUT);

    pinMode(BUZZER_PIN, OUTPUT);

    if (timeStatus() != timeSet)
        Serial.println("Unable to sync with the RTC");
    else
        Serial.println("RTC has set the system time");

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
        if (buzzer > now() && delta > 0)
            tone(BUZZER_PIN, 4000, 500);

        /* Some default setting for the display. */
        display.clearDisplay();
        display.setCursor(0,0);
        display.setTextColor(WHITE);

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

            display.fillRect(0, 0, maxWidth + 5, 64, BLACK);
            display.drawLine(maxWidth + 6, 0, maxWidth + 6, 64, WHITE);
            display.setTextSize(1);
            display.setCursor(0, 0);

            for (int i = 0; i < currentMenuLength; ++i) {
                if (i == currentMenuItem)
                    display.setTextColor(BLACK, WHITE);
                else
                    display.setTextColor(WHITE, BLACK);

                display.println(currentMenu[i].name);
            }
        }

        display.display();

        /* When buttons are pressed they set this to HIGH for feedback,
         * we set it to LOW again after the delay so it only lights up briefly. */
        digitalWriteFast(LED_BUILTIN, LOW);
    }
}

void WatchCore::setCurrentTime(time_t time) {
    Teensy3Clock.set(time);
    setTime(time);
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

    /* Poll the hall effect sensors for 8 milliseconds. */
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
            digitalWriteFast(LED_BUILTIN, HIGH);

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

    for (const WatchMenu* c = currentMenu; c->name != nullptr; ++c)
        currentMenuLength++;
}

void WatchCore::enableBuzzer(time_t seconds) {
    buzzer = now() + seconds;
}

namespace {

WatchMenu menu[] = {
    { "Clock", [](WatchMode* mode, WatchCore& core) {
          core.currentMode = WatchCore::Time;
          return true;
     }, nullptr, nullptr },
    { "Thermometer", [](WatchMode* mode, WatchCore& core) {
          core.currentMode = WatchCore::Tempurature;
          return true;
      }, nullptr, nullptr },
    { "Timer", [](WatchMode* mode, WatchCore& core) {
          core.currentMode = WatchCore::Timer;
          return true;
      }, nullptr, nullptr },
    { "Stopwatch", [](WatchMode* mode, WatchCore& core) {
          core.currentMode = WatchCore::Stopwatch;
          return true;
      }, nullptr, nullptr },
    { "Test", [](WatchMode* mode, WatchCore& core) {
          core.currentMode = WatchCore::Test;
          return true;
      }, nullptr, nullptr },
    { nullptr, nullptr, nullptr }
};

}

const WatchMenu modeMenu = { "Mode", nullptr, menu, nullptr };
