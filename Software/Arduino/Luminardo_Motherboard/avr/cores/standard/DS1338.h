/*
  DS1338.h - Real-Time Clock/Calendar library

  Copyright (c) 2014 Dmitry Pakhomenko.
  pahomenko@gmail.com
  http://magictale.com

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef DS1338_h
#define DS1338_h

#include <inttypes.h>
#include <Wire.h>

#if ARDUINO >= 100
#include <Arduino.h>       // for delayMicroseconds, digitalPinToBitMask, etc
#else
#include <WProgram.h>      // for delayMicroseconds
#include <pins_arduino.h>  // for digitalPinToBitMask, etc
#endif


#define DS1338_I2C_ADDR 0x68
#define DS1338_CH_BIT 7
#define DS1338_CTRL_REG_OUT 7
#define DS1338_CTRL_REG_OSF 5
#define DS1338_CTRL_REG_SQWE 4
#define DS1338_CTRL_REG_RS1 1
#define DS1338_CTRL_REG_RS0 0
#define DS1338_CTRL_REG_ADDR 0x7
#define DS1338_AM_PM_SELECTOR 6
#define DS1338_AM_PM_FLAG 5
#define DS1338_USR_RAM_START 0x8
#define DS1338_USR_RAM_END 0x3F

typedef struct struct_RtcTime
{	
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
	uint8_t day;
	uint8_t weekday;
	uint8_t month;
	uint16_t year;
} RtcTimeType;

class DS1338
{
  private:
        uint8_t _addr;
  public:
        DS1338(uint8_t i2cAddr);
	uint8_t init();
	uint8_t getTime(RtcTimeType* rtc);
	uint8_t setTime(RtcTimeType* rtc);
        void setClockHalt();
        void resetClockHalt();
        void setCtrlReg(uint8_t val);
        uint8_t getCtrlReg(uint8_t* val);

        uint8_t readBlock(uint8_t* buf, uint8_t startAddr, uint8_t len);
        void writeBlock(uint8_t* buf, uint8_t startAddr, uint8_t len);
};

extern DS1338 RTClock;

#endif

