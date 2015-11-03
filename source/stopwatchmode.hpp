#pragma once

#include <cstdint>
#include "watchmode.hpp"

class StopwatchMode : public WatchMode {
    uint32_t last;

public:
    StopwatchMode(WatchCore& c);

    virtual void draw(Adafruit_GFX& display);

    virtual void buttonPress(time_t buttonTime);

    virtual void left(uint8_t amount);
    virtual void right(uint8_t amount);
    virtual void up(uint8_t amount);
    virtual void down(uint8_t amount);

    virtual void tick(time_t delta);

    bool running;
    uint32_t length;
};
