/*
  HT16512.cpp - Driver for HT16512 VFD Controller
  Copyright (c) 2013 Dmitry Pakhomenko. 

  http://atmega.magictale.com

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

#include "settings.h"
#include "HT16515.h"

HT16515::HT16515()
{
}

void HT16515::init(uint8_t cs, uint8_t sclk, uint8_t data)
{
    _cs = cs;
    _sclk = sclk;
    _data = data;
}

void HT16515::reset(uint8_t mode) 
{
    pinMode(_cs, OUTPUT);
    pinMode(_sclk, OUTPUT);
    pinMode(_data, OUTPUT);
    digitalWrite(_cs, HIGH);
    digitalWrite(_sclk, HIGH);

    //Set display mode
    command(mode & HT16515_LOW_NIBBLES_SET, true);
    displayOnCmd(VFD_DIMMING_MAX);//maximum brightness

}

void HT16515::displayOnCmd(uint8_t dimming)
{
    command(HT16515_DISP_ON_CMD | (dimming & VFD_DIMMING_MAX), true);
}

void HT16515::displayOffCmd()
{
    command(HT16515_DISP_OFF_CMD, true);
}

void HT16515::displayMemWriteCmd(uint8_t addr_inc, uint8_t nodata)
{
    uint8_t cmd = HT16515_MEM_WR_CMD;
    if (addr_inc > 0) cmd | (1 << 2);
    command(cmd, nodata);
}

void HT16515::displayLEDWriteCmd(uint8_t addr_inc, uint8_t nodata)
{
    uint8_t cmd = HT16515_LED_WR_CMD;
    if (addr_inc > 0) cmd | (1 << 2);
    command(cmd, nodata);
}

void HT16515::addrSetCmd(uint8_t addr)
{
    command(HT16515_ADDR_SET_CMD | (addr & HT16515_ADDR_SET_MSK), true);
}

void HT16515::command(uint8_t value, uint8_t nodata) 
{
    digitalWrite(_sclk, HIGH);
    digitalWrite(_cs, LOW);

    uint8_t i;
    for (i = 0; i < HT16515_DATALEN; i++)
    {
        if (( value >> i ) & 0x1)
            digitalWrite(_data, HIGH);
        else 
            digitalWrite(_data, LOW);
        digitalWrite(_sclk, LOW);
        digitalWrite(_sclk, HIGH);
    }
    if (nodata > 0) digitalWrite(_cs, HIGH);
}

void HT16515::data(uint8_t value, uint8_t init_cs, uint8_t finalise_cs) 
{
    digitalWrite(_sclk, HIGH);
    if (init_cs > 0) digitalWrite(_cs, LOW);

    uint8_t i;
    for (i = 0; i < HT16515_DATALEN; i++)
    {
        if (( value >> i ) & 0x1)
            digitalWrite(_data, HIGH);
        else 
            digitalWrite(_data, LOW);
        digitalWrite(_sclk, LOW);
        digitalWrite(_sclk, HIGH);
    }
    if (finalise_cs > 0) digitalWrite(_cs, HIGH);
}




