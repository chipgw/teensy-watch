/*************************************************** 
  This is a library for the 1.5" & 1.27" 16-bit Color OLEDs
  with SSD1331 driver chip

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/1431
  ------> http://www.adafruit.com/products/1673

  These displays use SPI to communicate, 4 or 5 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include "Adafruit_SSD1351.h"
#ifdef __AVR
#include <avr/pgmspace.h>
#elif defined(ESP8266)
#include <pgmspace.h>
#endif
#include "pins_arduino.h"
#include "wiring_private.h"

#ifdef SPI4TEENSY3
#include <spi4teensy3.h>
#else
#include <SPI.h>
#endif

#ifndef _BV
#define _BV(bit) (1<<(bit))
#endif

/********************************** low level pin interface */

void Adafruit_SSD1351::writeCommand(uint8_t c) {
    *rsport &= ~ rspinmask;
    
    *csport &= ~ cspinmask;

#ifdef SPI4TEENSY3
    spi4teensy3::send(c);
#else
    SPI.transfer(c);
#endif
    
    *csport |= cspinmask;
}


void Adafruit_SSD1351::writeData(uint8_t c) {
    *rsport |= rspinmask;
    
    *csport &= ~ cspinmask;

#ifdef SPI4TEENSY3
    spi4teensy3::send(c);
#else
    SPI.transfer(c);
#endif
    
    *csport |= cspinmask;
} 

/***********************************/

uint16_t Adafruit_SSD1351::Color565(uint8_t r, uint8_t g, uint8_t b) {
    uint16_t c;
    c = r >> 3;
    c <<= 6;
    c |= g >> 2;
    c <<= 5;
    c |= b >> 3;

    return c;
}

void Adafruit_SSD1351::fillScreen(uint16_t fillcolor) {
    for (uint16_t y=0; y < SSD1351HEIGHT; ++y)
        for (uint16_t x=0; x < SSD1351WIDTH; ++x)
            buf[x][y] = fillcolor;
}

void Adafruit_SSD1351::clearScreen() {
    memset(buf, 0, sizeof(buf));
}

// Draw a filled rectangle with no rotation.
void Adafruit_SSD1351::update() {
    // set location
    writeCommand(SSD1351_CMD_SETCOLUMN);
    writeData(0);
    writeData(SSD1351WIDTH - 1);
    writeCommand(SSD1351_CMD_SETROW);
    writeData(0);
    writeData(SSD1351HEIGHT - 1);
    // fill!
    writeCommand(SSD1351_CMD_WRITERAM);

    for (uint16_t y=0; y < SSD1351HEIGHT; ++y) {
        for (uint16_t x=0; x < SSD1351WIDTH; ++x) {
            writeData(buf[x][y] >> 8);
            writeData(buf[x][y]);
        }
    }
}

void Adafruit_SSD1351::drawPixel(int16_t x, int16_t y, uint16_t color) {
    // Transform x and y based on current rotation.
    switch (getRotation()) {
    // Case 0: No rotation
    case 1:  // Rotated 90 degrees clockwise.
        swap(x, y);
        x = WIDTH - x - 1;
        break;
    case 2:  // Rotated 180 degrees clockwise.
        x = WIDTH - x - 1;
        y = HEIGHT - y - 1;
        break;
    case 3:  // Rotated 270 degrees clockwise.
        swap(x, y);
        y = HEIGHT - y - 1;
        break;
    }

    // Bounds check.
    if ((x >= SSD1351WIDTH) || (y >= SSD1351HEIGHT)) return;
    if ((x < 0) || (y < 0)) return;

    buf[x][y] = color;
}

void Adafruit_SSD1351::begin() {
    // set pin directions
    pinMode(_rs, OUTPUT);

    // using the hardware SPI
#ifdef SPI4TEENSY3
    spi4teensy3::init(1, SPI4TEENSY3_MODE_3);
#else
    SPI.begin();
    SPI.setClockDivider(SPI_CLOCK_DIV2);
    SPI.setDataMode(SPI_MODE3);
#endif

    // Toggle RST low to reset; CS low so it'll listen to us
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, LOW);
    
    if (_rst) {
        pinMode(_rst, OUTPUT);
        digitalWrite(_rst, HIGH);
        delay(100);
        digitalWrite(_rst, LOW);
        delay(100);
        digitalWrite(_rst, HIGH);
        delay(100);
    }

    // Initialization Sequence
    writeCommand(SSD1351_CMD_COMMANDLOCK);  // set command lock
    writeData(0x12);
    writeCommand(SSD1351_CMD_COMMANDLOCK);  // set command lock
    writeData(0xB1);

    writeCommand(SSD1351_CMD_DISPLAYOFF);  		// 0xAE

    writeCommand(SSD1351_CMD_CLOCKDIV);  		// 0xB3
    writeCommand(0xF1);  						// 7:4 = Oscillator Frequency, 3:0 = CLK Div Ratio (A[3:0]+1 = 1..16)
    
    writeCommand(SSD1351_CMD_MUXRATIO);
    writeData(127);
    
    writeCommand(SSD1351_CMD_SETREMAP);
    writeData(0x74);

    writeCommand(SSD1351_CMD_SETCOLUMN);
    writeData(0x00);
    writeData(0x7F);
    writeCommand(SSD1351_CMD_SETROW);
    writeData(0x00);
    writeData(0x7F);

    writeCommand(SSD1351_CMD_STARTLINE); 		// 0xA1

    writeData(SSD1351HEIGHT == 96 ? 96 : 0);


    writeCommand(SSD1351_CMD_DISPLAYOFFSET); 	// 0xA2
    writeData(0x0);

    writeCommand(SSD1351_CMD_SETGPIO);
    writeData(0x00);
    
    writeCommand(SSD1351_CMD_FUNCTIONSELECT);
    writeData(0x01); // internal (diode drop)
    //writeData(0x01); // external bias

//    writeCommand(SSSD1351_CMD_SETPHASELENGTH);
//    writeData(0x32);

    writeCommand(SSD1351_CMD_PRECHARGE);  		// 0xB1
    writeCommand(0x32);

    writeCommand(SSD1351_CMD_VCOMH);  			// 0xBE
    writeCommand(0x05);

    writeCommand(SSD1351_CMD_NORMALDISPLAY);  	// 0xA6

    writeCommand(SSD1351_CMD_CONTRASTABC);
    writeData(0xC8);
    writeData(0x80);
    writeData(0xC8);

    writeCommand(SSD1351_CMD_CONTRASTMASTER);
    writeData(0x0F);

    writeCommand(SSD1351_CMD_SETVSL );
    writeData(0xA0);
    writeData(0xB5);
    writeData(0x55);
    
    writeCommand(SSD1351_CMD_PRECHARGE2);
    writeData(0x01);
    
    writeCommand(SSD1351_CMD_DISPLAYON);		//--turn on oled panel
}

void  Adafruit_SSD1351::invert(boolean v) {
    writeCommand(v ? SSD1351_CMD_INVERTDISPLAY : SSD1351_CMD_NORMALDISPLAY);
}

/********************************* low level pin initialization */

Adafruit_SSD1351::Adafruit_SSD1351(uint8_t cs, uint8_t rs,  uint8_t rst) : Adafruit_GFX(SSD1351WIDTH, SSD1351HEIGHT) {
    _cs = cs;
    _rs = rs;
    _rst = rst;

    csport      = portOutputRegister(digitalPinToPort(cs));
    cspinmask   = digitalPinToBitMask(cs);
    
    rsport      = portOutputRegister(digitalPinToPort(rs));
    rspinmask   = digitalPinToBitMask(rs);
}

