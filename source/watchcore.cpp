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

    /* In case anything is still being shown on the display from last program run. */
    display.clearDisplay();
    display.display();

    Serial.println("init2");
    Serial.flush();
    digitalWriteFast(13, LOW);

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

    digitalWrite(TRACKBALL_WHT, HIGH);
    delay(200);
    digitalWrite(TRACKBALL_WHT, LOW);

    digitalWrite(TRACKBALL_GRN, HIGH);
    delay(200);
    digitalWrite(TRACKBALL_GRN, LOW);

    digitalWrite(TRACKBALL_RED, HIGH);
    delay(200);
    digitalWrite(TRACKBALL_RED, LOW);

    digitalWrite(TRACKBALL_BLU, HIGH);
    delay(200);
    digitalWrite(TRACKBALL_BLU, LOW);

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

        display.display();

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

    uint32_t lft = 0;
    uint32_t rgt = 0;
    uint32_t up  = 0;
    uint32_t dwn = 0;

    uint8_t lftNew = 0;
    uint8_t rgtNew = 0;
    uint8_t upNew  = 0;
    uint8_t dwnNew = 0;

    for(uint32_t start = micros(); (start + 1000) > micros();) {
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

    if (lft > 0 || rgt > 0)
        modes[currentMode]->buttonTwoPress(0);
}

void WatchCore::enableBuzzer(time_t seconds) {
    buzzer = now() + seconds;
}
