/*
  HT16515.h - Driver for HT16515 VFD Controller
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

#ifndef HT16515_H
#define HT16515_H

#define VFD_DIMMING_MAX 7

#define VFD_DISP_MODE_4D24S  0
#define VFD_DISP_MODE_5D23S  1
#define VFD_DISP_MODE_6D22S  2
#define VFD_DISP_MODE_7D21S  3
#define VFD_DISP_MODE_8D20S  4
#define VFD_DISP_MODE_9D19S  5
#define VFD_DISP_MODE_10D18S 6
#define VFD_DISP_MODE_11D17S 7
#define VFD_DISP_MODE_12D16S 8

#define HT16515_DISP_ON_CMD  0x88
#define HT16515_DISP_OFF_CMD 0x80
#define HT16515_MEM_WR_CMD   0x40
#define HT16515_LED_WR_CMD   0x41
#define HT16515_ADDR_SET_CMD 0xC0
#define HT16515_ADDR_SET_MSK 0x3F
#define HT16515_LOW_NIBBLES_SET 0x0F
#define HT16515_DATALEN 8


class HT16515
{
    public:
        HT16515();
        //Harware commands
        void init(uint8_t cs, uint8_t sclk, uint8_t data);
        void reset(uint8_t mode);
        void displayOnCmd(uint8_t dimming);
        void displayOffCmd();
        void displayMemWriteCmd(uint8_t addr_inc, uint8_t nodata);
        void displayLEDWriteCmd(uint8_t addr_inc, uint8_t nodata);
        //void displayMemReadCmd(uint8_t addr_inc, uint8_t nodata);
        void addrSetCmd(uint8_t addr);
        //Low level function for sending commands
        void command(uint8_t value, uint8_t nodata);
        //Low level function for sending data
        void data(uint8_t value, uint8_t init_cs, uint8_t finalise_cs);

protected: 
        uint8_t _cs, _sclk, _data;
};
#endif
