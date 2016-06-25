/*
  DS1338.cpp - Real-Time Clock/Calendar library

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

#include "DS1338.h"

#include <HardwareSerial.h>

// Initialize Class Variables //////////////////////////////////////////////////

// Constructors ////////////////////////////////////////////////////////////////

DS1338::DS1338(uint8_t addr)
{
  	_addr = addr;
}

// Public Methods //////////////////////////////////////////////////////////////
uint8_t DS1338::init()
{
	Wire.beginTransmission(_addr);
	return Wire.endTransmission();
}

uint8_t DS1338::getTime(RtcTimeType* rtc)
{
	Wire.beginTransmission(_addr);
	Wire.write(0);
	Wire.endTransmission();

	Wire.requestFrom(_addr, (uint8_t)7);

	uint8_t res = 0; 	

	if(7 <= Wire.available())
	{
		rtc->seconds = Wire.read() & 0x7F;
		rtc->minutes = Wire.read() & 0x7F;
		rtc->hours = Wire.read() & 0x3F;
		rtc->weekday = Wire.read() & 0x7;
		rtc->day = Wire.read() & 0x3F;
		rtc->month = Wire.read() & 0x1F;
		rtc->year = 0x2000 + Wire.read();
  	}else res = -1;

	return res;
}

uint8_t DS1338::setTime(RtcTimeType* rtc)
{
	Wire.beginTransmission(_addr);
        Wire.write(0);

	Wire.write(rtc->seconds & 0x7F);
	Wire.write(rtc->minutes & 0x7F);
	Wire.write(rtc->hours & 0x3F);
	Wire.write(rtc->weekday & 0x7);
	Wire.write(rtc->day & 0x3F);
	Wire.write(rtc->month & 0x1F);
	Wire.write((uint8_t)(rtc->year - 0x2000));
	return Wire.endTransmission();
}

void DS1338::setClockHalt()
{
	uint8_t temp;
        readBlock(&temp, 0, 1);
        temp |= 1 << DS1338_CH_BIT;
        writeBlock(&temp, 0, 1);
}

void DS1338::resetClockHalt()
{
	uint8_t temp;
        readBlock(&temp, 0, 1);
        temp &= ~(1 << DS1338_CH_BIT);
        writeBlock(&temp, 0, 1);
}

void DS1338::setCtrlReg(uint8_t val)
{
       writeBlock(&val, DS1338_CTRL_REG_ADDR, 1);
}

uint8_t DS1338::getCtrlReg(uint8_t* val)
{
        return readBlock(val, DS1338_CTRL_REG_ADDR, 1);
}

uint8_t DS1338::readBlock(uint8_t* buf, uint8_t startAddr, uint8_t len)
{
	Wire.beginTransmission(_addr);
	Wire.write(startAddr);
	Wire.endTransmission();

	Wire.requestFrom(_addr, len);
	uint8_t res = 0; 	
	if(len <= Wire.available())
	{
		while (len)
		{
			*buf++ = Wire.read();
			len--;
		}

 		res = 0;
        } else res = -1;
	return res;
}

void DS1338::writeBlock(uint8_t* buf, uint8_t startAddr, uint8_t len)
{
	Wire.beginTransmission(_addr);
        Wire.write(startAddr);

	while (len)
	{
		Wire.write(*buf++);
 		len--;
	}
	Wire.endTransmission();
}

// Preinstantiate Objects //////////////////////////////////////////////////////

DS1338 RTClock = DS1338(DS1338_I2C_ADDR);
