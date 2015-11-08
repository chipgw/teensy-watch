#pragma once

#include "watchmode.hpp"

class ClockMode : public WatchMode {
    int timeZone;

public:
    ClockMode(WatchCore& c);

    virtual void draw(Adafruit_GFX& display);

    virtual void buttonPress(time_t buttonTime);

    virtual void left(uint8_t amount);
    virtual void right(uint8_t amount);
    virtual void up(uint8_t amount);
    virtual void down(uint8_t amount);

    virtual void tick(time_t delta);

    bool setTime;
    time_t setTimeTime;
    time_t setTimeAmount;

    bool setTimeZone;

    bool analogMode;
};
