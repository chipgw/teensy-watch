#include "WProgram.h"
#include "watchcore.hpp"
#include "watchmode.hpp"
#include "clockmode.hpp"
#include "tempuraturemode.hpp"
#include "timermode.hpp"
#include "stopwatchmode.hpp"
#include <Wire.h>
#include <Time.h>

time_t getTeensy3Time() {
  return Teensy3Clock.get();
}

WatchCore::WatchCore() : display(2), currentMode(Time), buttonOneTime(0), buttonTwoTime(0) {
    setSyncProvider(getTeensy3Time);
    pinMode(13, OUTPUT);

    digitalWriteFast(13, HIGH);
    delay(100);

    modes[Time]         = new ClockMode(*this);
    modes[Tempurature]  = new TempuratureMode(*this);
    modes[Timer]        = new TimerMode(*this);
    modes[Stopwatch]    = new StopwatchMode(*this);

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

    display.clearDisplay();

    Serial.println("init2");
    Serial.flush();
    digitalWriteFast(13, LOW);

    pinMode(BUTTON_ONE, INPUT);
    pinMode(BUTTON_TWO, INPUT);
    pinMode(BUZZER_PIN, OUTPUT);

    if (timeStatus() != timeSet)
        Serial.println("Unable to sync with the RTC");
    else
        Serial.println("RTC has set the system time");
}

WatchCore::~WatchCore() {
    for (WatchMode* mode : modes)
        delete mode;
}

void WatchCore::run() {
    Serial.println("run");

    time_t last = now();

    while (true) {
        time_t delta = now() - last;
        last = now();

        doInput();

        bool buzzer = false;

        for (WatchMode* mode : modes) {
            mode->tick(delta);

            if (mode->isBuzzer())
                buzzer = true;
        }

        if (buzzer && millis() % 1000 > 500)
            tone(BUZZER_PIN, 4000);
        else
            noTone(BUZZER_PIN);

        display.clearDisplay();
        display.setCursor(0,0);
        display.setTextColor(WHITE);

        modes[currentMode]->draw(display);

        display.display();

        delay(50);
        digitalWriteFast(13, LOW);
    }
}

void WatchCore::doInput() {
    while (Serial.available()) {
        String data = Serial.readStringUntil(':');

        /* Try getting the time from serial. */
        if (data.endsWith("Time")) {
            const time_t DEFAULT_TIME = 1357041600;

            time_t t = Serial.parseInt();

            if (t > DEFAULT_TIME) {
                Teensy3Clock.set(t);
                setTime(t);
            }
        }
    }

    /* LOW is pressed. */
    if (digitalRead(BUTTON_ONE) == LOW) {
        if (buttonOneTime == 0)
            buttonOneTime = now();
    } else {
        if(buttonOneTime != 0) {
            digitalWriteFast(13, HIGH);
            modes[currentMode]->buttonOnePress(now() - buttonOneTime);
        }

        buttonOneTime = 0;
    }
    if (digitalRead(BUTTON_TWO) == LOW) {
        if (buttonTwoTime == 0)
            buttonTwoTime = now();
    } else {
        if(buttonTwoTime != 0) {
            digitalWriteFast(13, HIGH);
            modes[currentMode]->buttonTwoPress(now() - buttonTwoTime);
        }

        buttonTwoTime = 0;
    }
}
