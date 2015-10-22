#pragma once

#include "watchmode.hpp"

class ClockMode : public WatchMode {
public:
    ClockMode(WatchCore& c);

    virtual void draw(Adafruit_GFX& display);

    virtual void buttonOnePress(time_t buttonTime);
    virtual void buttonTwoPress(time_t buttonTime);

    virtual bool isBuzzer();

    virtual void tick(time_t delta);
};
