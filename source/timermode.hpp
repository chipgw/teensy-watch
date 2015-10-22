#pragma once

#include "watchmode.hpp"

class TimerMode : public WatchMode {
    /* The full amount of time the timer is set to. */
    time_t setting;

    /* The amount of time that remains in the running timer. */
    time_t remaining;

    /* Whether or not the timer is running. */
    bool running;

public:
    TimerMode(WatchCore& c);

    virtual void draw(Adafruit_GFX& display);

    virtual void buttonOnePress(time_t buttonTime);
    virtual void buttonTwoPress(time_t buttonTime);

    virtual bool isBuzzer();

    virtual void tick(time_t delta);
};
