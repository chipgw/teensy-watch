#include "WProgram.h"
#include "watchcore.hpp"
#include <Wire.h>
#include <TimeAlarms.h>

time_t getTeensy3Time() {
  return Teensy3Clock.get();
}

WatchCore::WatchCore() : display(2), mode(Time), buttonOneTime(0), buttonTwoTime(0), timerStart(0), timerLength(10) {
    setSyncProvider(getTeensy3Time);
    pinMode(13, OUTPUT);

    digitalWriteFast(13, HIGH);
    delay(100);

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

void WatchCore::run() {
    Serial.println("run");
    while (true) {
        doInput();

        if (timerStart > 0 && (timerStart + timerLength - now()) < 0 && now() % 2)
            tone(BUZZER_PIN, 2000);
        else
            noTone(BUZZER_PIN);

        display.clearDisplay();
        display.setCursor(0,0);
        display.setTextColor(WHITE);

        switch (mode) {
        case Time:
            /* buttonOneTime is 0 when button is not pressed. */
            if(buttonOneTime != 0) {
                time_t buttonOneDelta = now() - buttonOneTime;
                if (buttonOneDelta > SET_PRESS_TIME) {
                    mode = TimeSet;
                    break;
                } else if (buttonOneDelta > (SET_PRESS_TIME / 2)) {
                    display.setTextSize(3);
                    display.print("Hold to SET");
                    break;
                }
            }

        case TimeSet:
            drawTime();
            break;
        case Timer:
            drawTimer();
            break;
        case Tempurature:
            drawTempurature();
            break;
        case Stopwatch:
            break;
        default:
            break;
        }
        display.display();

        Alarm.delay(50);
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
        if(buttonOneTime != 0)
            switchMode(now() - buttonOneTime);

        buttonOneTime = 0;
    }
    if (digitalRead(BUTTON_TWO) == LOW) {
        if (buttonTwoTime == 0)
            buttonTwoTime = now();
    } else {
        if(buttonTwoTime != 0)
            timerStart = timerStart == 0 ? now() : 0;

        buttonTwoTime = 0;
    }
}

void WatchCore::switchMode(time_t buttonTime) {
    switch (mode) {
    case Time:
        if(buttonTime > (SET_PRESS_TIME / 2))
            break;
        mode = Tempurature;
        break;
    case TimeSet:
        if(buttonTime < SET_PRESS_TIME)
            mode = Time;
        break;
    case Tempurature:
        mode = Timer;
        break;
    case Timer:
        mode = Stopwatch;
        break;
    case Stopwatch:
    default:
        mode = Time;
        break;
    }
    digitalWriteFast(13, HIGH);
}

void WatchCore::drawTime() {
    display.setTextSize(3);
    display.printf("%02i:%02i", hour(), minute());
    display.setTextSize(2);
    display.printf(":%02i", second());
    display.println();
}

void WatchCore::drawTimer() {
    time_t remaining = timerStart > 0 ? max(0, timerStart + timerLength - now()) : timerLength;

    display.setTextSize(3);
    display.printf("%02i:%02i", hour(remaining), minute(remaining));
    display.setTextSize(2);
    display.printf(":%02i", second(remaining));
    display.println();
}

void WatchCore::drawTempurature() {
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
