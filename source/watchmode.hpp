#pragma once

#include <sys/types.h>

class WatchCore;
class Adafruit_GFX;

class WatchMode {
protected:
    WatchCore& core;

public:
    WatchMode(WatchCore& c) : core(c) { }
    virtual ~WatchMode() = default;

    virtual void draw(Adafruit_GFX& display) = 0;

    virtual void buttonOnePress(time_t buttonTime) = 0;
    virtual void buttonTwoPress(time_t buttonTime) = 0;

    virtual void tick(time_t delta) = 0;
};
