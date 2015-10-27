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

WatchCore::WatchCore() : display(2), buttonOneTime(0), buttonTwoTime(0), currentMode(Time) {
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

        display.display();

        delay(10);

        /* When buttons are pressed they set this to HIGH for feedback,
         * we set it to LOW again after the delay so it only lights up briefly. */
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
                last = t;
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

            /* If the buzzer is enabled all pressing a button does is stop it. */
            if (buzzer > now())
                buzzer = 0;
            else
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

            /* If the buzzer is enabled all pressing a button does is stop it. */
            if (buzzer > now())
                buzzer = 0;
            else
                modes[currentMode]->buttonTwoPress(now() - buttonTwoTime);
        }

        buttonTwoTime = 0;
    }
}

void WatchCore::enableBuzzer(time_t seconds) {
    buzzer = now() + seconds;
}
