#include "clockmode.hpp"
#include "watchcore.hpp"
#include <Time.h>
#include <Adafruit_GFX.h>
#include <Timezone.h>

#define ANALOG_CENTER_X 64
#define ANALOG_CENTER_Y 32

#define SECOND_HAND_LENGTH 31
#define MINUTE_HAND_LENGTH 24
#define HOUR_HAND_LENGTH 16

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

ClockMode::ClockMode(WatchCore& c) : WatchMode(c), timeZone(0), setTimeZone(false), analogMode(false) { }

void ClockMode::draw(Adafruit_GFX& display) {
    time_t local = zones[timeZone].zone.toLocal(now());

    if (analogMode) {
        display.drawCircle(ANALOG_CENTER_X, ANALOG_CENTER_Y, SECOND_HAND_LENGTH, WHITE);

        double secondHand = (1.0 - (second(local) / 30.0)) * M_PI;
        double minuteHand = (1.0 - (minute(local) / 30.0)) * M_PI;
        double hourHand = (1.0 - (hour(local) / 6.0)) * M_PI;

        display.drawLine(ANALOG_CENTER_X, ANALOG_CENTER_Y, ANALOG_CENTER_X + sin(secondHand) * SECOND_HAND_LENGTH, ANALOG_CENTER_Y + cos(secondHand) * SECOND_HAND_LENGTH, WHITE);
        display.drawLine(ANALOG_CENTER_X, ANALOG_CENTER_Y, ANALOG_CENTER_X + sin(minuteHand) * MINUTE_HAND_LENGTH, ANALOG_CENTER_Y + cos(minuteHand) * MINUTE_HAND_LENGTH, WHITE);
        display.drawLine(ANALOG_CENTER_X, ANALOG_CENTER_Y, ANALOG_CENTER_X + sin(hourHand) * HOUR_HAND_LENGTH, ANALOG_CENTER_Y + cos(hourHand) * HOUR_HAND_LENGTH, WHITE);

        if (setTimeZone) {
            display.setTextColor(BLACK, WHITE);

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
            display.setTextColor(BLACK, WHITE);

        display.print(zones[timeZone].name);
    }
}

namespace {

WatchMenu menu[] = {
    { "Toggle Analog", [](WatchMode* mode, WatchCore& core) {
          ClockMode* clock = static_cast<ClockMode*>(mode);

          if (clock)
              clock->analogMode = !clock->analogMode;

          return true;
     }, nullptr },
    { "Set Time Zone", [](WatchMode* mode, WatchCore& core) {
          ClockMode* clock = static_cast<ClockMode*>(mode);

          if (clock)
          clock->setTimeZone = true;

          return true;
      }, nullptr },
    { "Set Time", [](WatchMode* mode, WatchCore& core) {
          /* TODO - Implement. */
          return true;
      }, nullptr },
    modeMenu,
    { nullptr, nullptr, nullptr }
};

}

void ClockMode::buttonPress(time_t buttonTime) {
    if (setTimeZone)
        setTimeZone = false;
    else
        core.openMenu(menu);
}

void ClockMode::left(uint8_t amount) {
    if (setTimeZone && (timeZone -= amount) < 0)
        timeZone = numZones - 1;
}

void ClockMode::right(uint8_t amount) {
    if (setTimeZone && (timeZone += amount) >= numZones)
        timeZone = 0;
}

void ClockMode::up(uint8_t amount) { }

void ClockMode::down(uint8_t amount) { }

void ClockMode::tick(time_t delta) { }
