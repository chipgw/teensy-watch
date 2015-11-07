#pragma once

#include "watchmode.hpp"

class TimerMode : public WatchMode {

public:
    TimerMode(WatchCore& c);

    virtual void draw(Adafruit_GFX& display);

    virtual void buttonPress(time_t buttonTime);

    virtual void left(uint8_t amount);
    virtual void right(uint8_t amount);
    virtual void up(uint8_t amount);
    virtual void down(uint8_t amount);

    virtual void tick(time_t delta);

    /* The full amount of time the timer is set to. */
    time_t setting;

    /* The amount of time that remains in the running timer. */
    time_t remaining;

    /* Whether or not the timer is running. */
    bool running;

    bool setTimer;
    time_t setAmount;
};
