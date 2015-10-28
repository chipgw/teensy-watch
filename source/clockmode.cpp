#include "clockmode.hpp"
#include "watchcore.hpp"
#include <Time.h>
#include <Adafruit_GFX.h>
#include <Timezone.h>

struct Zone {
    const char* name;
    Timezone zone;
};

Zone zones[] = {
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

ClockMode::ClockMode(WatchCore& c) : WatchMode(c), timeZone(0) { }

void ClockMode::draw(Adafruit_GFX& display) {
    time_t local = zones[timeZone].zone.toLocal(now());

    display.setTextSize(3);
    display.printf("%02i:%02i", hour(local), minute(local));
    display.setTextSize(2);
    display.printf(":%02i", second(local));

    display.setCursor(0, 24);
    display.setTextSize(1);
    display.printf("%s, %04i-%02i-%02i", dayStr(dayOfWeek(local)), year(local), month(local), day(local));

    display.setCursor(0, 32);
    display.print(zones[timeZone].name);
}

void ClockMode::buttonOnePress(time_t buttonTime) {
    core.currentMode = WatchCore::Tempurature;
}

void ClockMode::buttonTwoPress(time_t buttonTime) {
    if (++timeZone >= numZones)
        timeZone = 0;
}

void ClockMode::tick(time_t delta) {

}
