cmake_minimum_required(VERSION 3.0)
project(teensy-watch)

teensy_create_target(${PROJECT_NAME})

teensy_link_libraries(${PROJECT_NAME} Time Timezone Wire SPI Adafruit_GFX Adafruit_SSD1306)

file(GLOB SOURCES "source/*.cpp")

teensy_add_executable(${PROJECT_NAME} ${SOURCES})

add_definitions(-DARDUINO=160 -DTEENSYDUINO=102)
