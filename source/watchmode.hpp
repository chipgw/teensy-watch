#pragma once

#include <cstdint>
#include <sys/types.h>

class WatchMode;
class WatchCore;
class Adafruit_GFX;

struct WatchMenu {
    const char* name;

    bool (*callback)(WatchMode*, WatchCore&);

    const WatchMenu* subMenu;

    mutable const WatchMenu* previousMenu;
};

const extern WatchMenu modeMenu;
const extern WatchMenu lightMenu;

class WatchMode {
protected:
    WatchCore& core;

public:
    WatchMode(WatchCore& c) : core(c) { }
    virtual ~WatchMode() = default;

    virtual void draw(Adafruit_GFX& display) = 0;

    virtual void buttonPress(time_t buttonTime) = 0;

    virtual void left(uint8_t amount) = 0;
    virtual void right(uint8_t amount) = 0;
    virtual void up(uint8_t amount) = 0;
    virtual void down(uint8_t amount) = 0;

    virtual void tick(time_t delta) = 0;
};
