cmake_minimum_required(VERSION 3.0)
project(teensy-watch)

option(SPI4TEENSY3 "Use spi4teensy3 instead of normal SPI library for communicating with the OLED screen." OFF)

teensy_create_target(${PROJECT_NAME})
if(SPI4TEENSY3)
    teensy_link_libraries(${PROJECT_NAME} spi4teensy3)
    add_definitions(-DSPI4TEENSY3)
else(SPI4TEENSY3)
    teensy_link_libraries(${PROJECT_NAME} SPI)
endif(SPI4TEENSY3)

teensy_link_libraries(${PROJECT_NAME} Time Timezone Wire Adafruit_GFX Adafruit_SSD1351)

file(GLOB SOURCES "source/*.cpp")

teensy_add_executable(${PROJECT_NAME} ${SOURCES})

add_definitions(-DARDUINO=160 -DTEENSYDUINO=102)
