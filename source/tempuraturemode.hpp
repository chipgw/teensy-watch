#pragma once

#include "watchmode.hpp"

class TempuratureMode : public WatchMode {
    int mV;
    int temperatureC;
    int temperatureF;

public:
    TempuratureMode(WatchCore& c);

    virtual void draw(Adafruit_GFX& display);

    virtual void buttonPress(time_t buttonTime);

    virtual void left(uint8_t amount);
    virtual void right(uint8_t amount);
    virtual void up(uint8_t amount);
    virtual void down(uint8_t amount);

    virtual void tick(time_t delta);

    bool freeze;
};
