#!/bin/bash
# Replace /dev/ttyACM0 with path of the teensy serial port. This is what it is on both my Arch Linux devices.
date +Time:%s > /dev/ttyACM0
