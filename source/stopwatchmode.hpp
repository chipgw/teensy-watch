#pragma once

#include <cstdint>
#include "watchmode.hpp"

class StopwatchMode : public WatchMode {
    bool running;
    uint32_t length;
    uint32_t last;

public:
    StopwatchMode(WatchCore& c);

    virtual void draw(Adafruit_GFX& display);

    virtual void buttonOnePress(time_t buttonTime);
    virtual void buttonTwoPress(time_t buttonTime);

    virtual bool isBuzzer();

    virtual void tick(time_t delta);
};
