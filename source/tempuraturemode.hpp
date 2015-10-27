#pragma once

#include "watchmode.hpp"

class TempuratureMode : public WatchMode {
public:
    TempuratureMode(WatchCore& c);

    virtual void draw(Adafruit_GFX& display);

    virtual void buttonOnePress(time_t buttonTime);
    virtual void buttonTwoPress(time_t buttonTime);

    virtual void tick(time_t delta);
};
