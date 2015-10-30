#!/bin/bash
# Replace /dev/ttyACM0 with path of the teensy serial port. This is what it is on both my Arch Linux devices.
echo "Time:$(date +%s)" > /dev/ttyACM0
