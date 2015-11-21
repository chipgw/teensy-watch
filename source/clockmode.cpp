#include "clockmode.hpp"
#include "watchcore.hpp"
#include <Time.h>
#include <Timezone.h>

#include "watch-bg.xbm"

struct Zone {
    const char* name;
    Timezone zone;
};

Zone zones[] = {
    { "UTC", Timezone(
      TimeChangeRule{"UTC", First, Sun, Oct, 2, 0},
      TimeChangeRule{"UTC", First, Sun, Oct, 2, 0})
    },
    //Australia Eastern Time Zone (Sydney, Melbourne)
    { "Australia Eastern", Timezone(
      TimeChangeRule{"AEDT", First, Sun, Oct, 2, 660},    //UTC + 11 hours
      TimeChangeRule{"AEST", First, Sun, Apr, 3, 600})    //UTC + 10 hours
    },
    //Central European Time (Frankfurt, Paris)
    { "Central European", Timezone(
      TimeChangeRule{"CEST", Last, Sun, Mar, 2, 120},     //Central European Summer Time
      TimeChangeRule{"CET ", Last, Sun, Oct, 3, 60})       //Central European Standard Time
    },
    //United Kingdom (London, Belfast)
    { "United Kingdom", Timezone(
      TimeChangeRule{"BST", Last, Sun, Mar, 1, 60},        //British Summer Time
      TimeChangeRule{"GMT", Last, Sun, Oct, 2, 0})         //Standard Time
    },
    //US Eastern Time Zone (New York, Detroit)
    { "US Eastern", Timezone(
      TimeChangeRule{"EDT", Second, Sun, Mar, 2, -240},  //Eastern Daylight Time = UTC - 4 hours
      TimeChangeRule{"EST", First, Sun, Nov, 2, -300})   //Eastern Standard Time = UTC - 5 hours
    },
    //US Central Time Zone (Chicago, Houston)
    { "US Central", Timezone(
      TimeChangeRule{"CDT", Second, Sun, Mar, 2, -300},
      TimeChangeRule{"CST", First, Sun, Nov, 2, -360})
    },
    //US Mountain Time Zone (Denver, Salt Lake City)
    { "US Mountain", Timezone(
      TimeChangeRule{"MDT", Second, Sun, Mar, 2, -360},
      TimeChangeRule{"MST", First, Sun, Nov, 2, -420})
    },
    //Arizona is US Mountain Time Zone but does not use DST
    { "Arizona", Timezone(
      TimeChangeRule{"MST", First, Sun, Nov, 2, -420},
      TimeChangeRule{"MST", First, Sun, Nov, 2, -420})
    },
    //US Pacific Time Zone (Las Vegas, Los Angeles)
    { "US Pacific", Timezone(
      TimeChangeRule{"PDT", Second, Sun, Mar, 2, -420},
      TimeChangeRule{"PST", First, Sun, Nov, 2, -480})
    },
//    { "", Timezone(
//      ,
//      )
//    },
};

constexpr int numZones = sizeof(zones) / sizeof(zones[0]);

ClockMode::ClockMode(WatchCore& c) : WatchMode(c), timeZone(0), setTime(false), setTimeAmount(1), setTimeZone(false), analogMode(false) { }

void ClockMode::draw(Adafruit_GFX& display) {
    time_t local = zones[timeZone].zone.toLocal(now());

    if (setTime) {
        local = zones[timeZone].zone.toLocal(setTimeTime);

        bool blinkOff = (millis() % 1000) < 500;
        display.setTextSize(3);
        if (!blinkOff || setTimeAmount != SECS_PER_HOUR)
            display.printf("%02i", hour(local));
        else
            display.print("  ");

        if (!blinkOff || setTimeAmount != SECS_PER_MIN)
            display.printf(":%02i", minute(local));
        else
            display.print(":  ");

        display.setTextSize(2);
        if (!blinkOff || setTimeAmount != 1)
            display.printf(":%02i", second(local));
        else
            display.print(":  ");

        display.setCursor(0, 24);
        display.setTextSize(1);
        display.printf("%s, ", dayStr(dayOfWeek(local)));

        if (!blinkOff || setTimeAmount != SECS_PER_YEAR)
            display.printf("%04i", year(local));
        else
            display.print("    ");

        if (!blinkOff || setTimeAmount != SECS_PER_DAY * 30)
            display.printf("-%02i", month(local));
        else
            display.print("-  ");

        if (!blinkOff || setTimeAmount != SECS_PER_DAY)
            display.printf("-%02i", day(local));
        else
            display.print("-  ");

        display.setCursor(0, 32);
        display.print(zones[timeZone].name);
    } else if (analogMode) {
        /* For some reason the xbm draws the specified color in the pixels that are supposed to be BLACK.
         * So WHITE is the "background color" that will fill where it's supposed to be because black gets drawn everywhere else. */
        display.fillScreen(WHITE);
        display.drawXBitmap(0, 0, watch_bg_bits, watch_bg_width, watch_bg_height, BLACK);

        double secondHand = (1.0 - (second(local) / 30.0)) * M_PI;
        double minuteHand = (1.0 - (minute(local) / 30.0)) * M_PI;
        double hourHand = (1.0 - (hour(local) / 6.0)) * M_PI;

        /* Find the center of the screen. */
        int16_t centerX = display.width() / 2;
        int16_t centerY = display.height() / 2;

        /* The second hand length is equal to whichever is center coordinate is smaller. The other hands are a fraction of the second hand length. */
        int16_t secondHandLen = centerX < centerY ? centerX: centerY;
        int16_t minuteHandLen = secondHandLen * 3 / 4;
        int16_t hourHandLen = secondHandLen / 2;

        display.drawLine(centerX, centerY, centerX + sin(secondHand) * secondHandLen, centerY + cos(secondHand) * secondHandLen, WHITE);
        display.drawLine(centerX, centerY, centerX + sin(minuteHand) * minuteHandLen, centerY + cos(minuteHand) * minuteHandLen, WHITE);
        display.drawLine(centerX, centerY, centerX + sin(hourHand)   * hourHandLen,   centerY + cos(hourHand)   * hourHandLen,   WHITE);

        if (setTimeZone) {
            display.setTextColor(BLACK, RED);

            display.print(zones[timeZone].name);
        }
    } else {
        display.setTextSize(3);
        display.printf("%02i:%02i", hour(local), minute(local));
        display.setTextSize(2);
        display.printf(":%02i", second(local));

        display.setCursor(0, 24);
        display.setTextSize(1);
        display.printf("%s, %04i-%02i-%02i", dayStr(dayOfWeek(local)), year(local), month(local), day(local));

        display.setCursor(0, 32);

        if (setTimeZone)
            display.setTextColor(BLACK, RED);

        display.print(zones[timeZone].name);
    }
}

namespace {

const WatchMenu menu[] = {
    { "Toggle Analog", [](WatchMode* mode, WatchCore& core) {
          ClockMode* clock = static_cast<ClockMode*>(mode);

          if (clock)
              clock->analogMode = !clock->analogMode;

          return true;
     }},
    { "Set Time Zone", [](WatchMode* mode, WatchCore& core) {
          ClockMode* clock = static_cast<ClockMode*>(mode);

          if (clock)
              clock->setTimeZone = true;

          return true;
      }},
    { "Set Time", [](WatchMode* mode, WatchCore& core) {
          ClockMode* clock = static_cast<ClockMode*>(mode);

          if (clock) {
              clock->setTime = true;
              clock->setTimeTime = now();
          }

          return true;
      }},
    { lightMenu.name, lightMenu.callback },
    modeMenu,
    { nullptr }
};

}

void ClockMode::buttonPress(time_t buttonTime) {
    if (setTime) {
        setTime = false;
        core.setCurrentTime(setTimeTime);
    } else if (setTimeZone)
        setTimeZone = false;
    else
        core.openMenu(menu);
}

void ClockMode::left(uint8_t amount) {
    if (setTime) {
        if (setTimeAmount == SECS_PER_HOUR)
            setTimeAmount = SECS_PER_DAY;
        else if (setTimeAmount == SECS_PER_DAY)
            setTimeAmount = SECS_PER_DAY * 30;
        else if (setTimeAmount == SECS_PER_DAY * 30)
            setTimeAmount = SECS_PER_YEAR;
        else if (setTimeAmount == SECS_PER_YEAR)
            setTimeAmount = 1;
        else
            setTimeAmount *= 60;
    }
    if (setTimeZone && (timeZone -= amount) < 0)
        timeZone = numZones - 1;
}

void ClockMode::right(uint8_t amount) {
    if (setTime) {
        if (setTimeAmount == SECS_PER_YEAR)
            setTimeAmount = SECS_PER_DAY * 30;
        else if (setTimeAmount == SECS_PER_DAY * 30)
            setTimeAmount = SECS_PER_DAY;
        else if (setTimeAmount == SECS_PER_DAY)
            setTimeAmount = SECS_PER_HOUR;
        else if (setTimeAmount == 1)
            setTimeAmount = SECS_PER_YEAR;
        else
            setTimeAmount /= 60;
    }
    if (setTimeZone && (timeZone += amount) >= numZones)
        timeZone = 0;
}

void ClockMode::up(uint8_t amount) {
    if (setTime)
        setTimeTime += amount * setTimeAmount;
}

void ClockMode::down(uint8_t amount) {
    if (setTime)
        setTimeTime -= amount * setTimeAmount;
}

void ClockMode::tick(time_t delta) {
    if (setTime)
        setTimeTime += delta;
}
