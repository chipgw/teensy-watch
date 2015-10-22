# This toolchain file is based on https://github.com/apmorton/teensy-template/blob/master/Makefile
# and on the Teensy Makefile.

include(CMakeForceCompiler)


# User configurable variables
# (show up in cmake-gui)
#
set(TEENSY_VERSION 31 CACHE STRING "Set to the Teensy version corresponding to your board (30 or 31 allowed)" FORCE)
set(CPU_CORE_SPEED 96000000 CACHE STRING "Set to 24000000, 48000000, 72000000 or 96000000 to set CPU core speed" FORCE)

if(APPLE)
    set(ARDUINOPATH "/Applications/Arduino.app/Contents/Resources/Java/" CACHE STRING "Path to Arduino installation")
elseif(UNIX)
    set(ARDUINOPATH "/usr/share/arduino/" CACHE STRING "Path to Arduino installation")
endif()


# Derived variables
#
set(CPU cortex-m4)

set(TOOLSPATH "${ARDUINOPATH}hardware/tools/")
set(COMPILERPATH "${TOOLSPATH}arm/bin/")
set(COREPATH "${ARDUINOPATH}hardware/teensy/avr/cores/teensy3/")


# Normal toolchain configuration
#

# this one is important
set(CMAKE_SYSTEM_NAME Generic)

# specify the cross compiler
cmake_force_c_compiler(${COMPILERPATH}arm-none-eabi-gcc GNU)
cmake_force_cxx_compiler(${COMPILERPATH}arm-none-eabi-g++ GNU)

# where is the target environment 
set(CMAKE_FIND_ROOT_PATH ${COMPILERPATH})

# search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)


# Additional C and CPP files for Arduino support
#

file(GLOB TEENSY_C_FILES ${COREPATH}*.c)
file(GLOB TEENSY_CPP_FILES ${COREPATH}*.cpp)
set(TEENSY_SOURCES ${TEENSY_C_FILES} ${TEENSY_CPP_FILES})
list(REMOVE_ITEM TEENSY_SOURCES ${COREPATH}main.cpp)


# Teensy 3.0 and 3.1 differentiation
#

if (TEENSY_VERSION EQUAL 30)
    set(CPU_DEFINE __MK20DX128__)
    set(LINKER_FILE ${COREPATH}mk20dx128.ld)
else()
    set(CPU_DEFINE __MK20DX256__)
    set(LINKER_FILE ${COREPATH}mk20dx256.ld)
endif()

set(TEENSY_DEFINITIONS USB_SERIAL
                       LAYOUT_US_ENGLISH
                       F_CPU=${CPU_CORE_SPEED}
                       ${CPU_DEFINE})


# Macros to wrap add_[executable|library] for seamless Teensy integration
#
function(teensy_add_executable TARGET)
    set(ELFTARGET ${TARGET}.elf)

    add_executable(${ELFTARGET} ${ARGN} ${${TARGET}_LIBRARY_SOURCES} ${TEENSY_SOURCES})

    set_target_properties(${ELFTARGET} PROPERTIES COMPILE_FLAGS "-Wall -g -Os -mcpu=${CPU} -mthumb -nostdlib -MMD -felide-constructors -fno-exceptions -fno-rtti -std=gnu++0x")
    set_target_properties(${ELFTARGET} PROPERTIES COMPILE_DEFINITIONS "${TEENSY_DEFINITIONS}")
    set_target_properties(${ELFTARGET} PROPERTIES INCLUDE_DIRECTORIES "${COREPATH}")
    set_target_properties(${ELFTARGET} PROPERTIES LINK_FLAGS "-Os -Wl,--gc-sections,--defsym=__rtc_localtime=0 --specs=nano.specs -mcpu=${CPU} -mthumb -T${LINKER_FILE}")

    add_custom_command(OUTPUT ${TARGET}.hex
                       COMMAND ${COMPILERPATH}arm-none-eabi-size ${ELFTARGET}
                       COMMAND ${COMPILERPATH}arm-none-eabi-objcopy -O ihex -R .eeprom ${ELFTARGET} ${TARGET}.hex
                       COMMAND ${TOOLSPATH}teensy_post_compile -file=${TARGET} -path=${CMAKE_CURRENT_BINARY_DIR} -tools=${TOOLSPATH}
                       DEPENDS ${ELFTARGET}
                       COMMENT "Creating HEX file for ${ELFTARGET}")

    add_custom_target(hex ALL DEPENDS ${TARGET}.hex)

    include_directories(${${TARGET}_LIBRARY_INCLUDE_DIRS})
endfunction()

function(teensy_create_target TARGET)
    unset(${TARGET}_LIBRARY_SOURCES CACHE)
    unset(${TARGET}_LIBRARY_INCLUDE_DIRS CACHE)
endfunction()

# TODO - This might not always work...
set(USER_HOME $ENV{HOME})

# Should be called *before* calling teensy_add_executable.
function(teensy_link_libraries TARGET)
    foreach(ARG ${ARGN})
        if(EXISTS "${ARG}")
            set(LIBRARY_PATH "${ARG}")
        elseif(EXISTS "${ARDUINOPATH}/hardware/teensy/avr/libraries/${ARG}")
            set(LIBRARY_PATH "${ARDUINOPATH}/hardware/teensy/avr/libraries/${ARG}")
        elseif(EXISTS "${ARDUINOPATH}/libraries/${ARG}")
            set(LIBRARY_PATH "${ARDUINOPATH}/libraries/${ARG}")
        elseif(EXISTS "${USER_HOME}/Arduino/libraries/${ARG}")
            set(LIBRARY_PATH "${USER_HOME}/Arduino/libraries/${ARG}")
        else()
            # TODO - Support other search paths.
            message(FATAL_ERROR "Unable to find the ${ARG} library directory!")
        endif()

        # Add all source files to the directory. */
        file(GLOB_RECURSE LIBRARY_SOURCES "${LIBRARY_PATH}/*.cpp" "${LIBRARY_PATH}/*.c" "${LIBRARY_PATH}/*.cc")

        set(${TARGET}_LIBRARY_SOURCES ${${TARGET}_LIBRARY_SOURCES} ${LIBRARY_SOURCES} CACHE INTERNAL "" FORCE)

        # Find headers in library folder and add to inlude paths.
        file(GLOB_RECURSE LIBRARY_HEADERS "${LIBRARY_PATH}/*.h")
        set(LIBRARY_INCLUDE_DIRS "")
        foreach(HEADER ${LIBRARY_HEADERS})
            get_filename_component(DIR ${HEADER} PATH)
            list(APPEND LIBRARY_INCLUDE_DIRS "${DIR}/")
        endforeach()
        list(REMOVE_DUPLICATES LIBRARY_INCLUDE_DIRS)

        set(${TARGET}_LIBRARY_INCLUDE_DIRS ${${TARGET}_LIBRARY_INCLUDE_DIRS} ${LIBRARY_INCLUDE_DIRS} CACHE INTERNAL "" FORCE)
    endforeach()
endfunction()
