/*
  Charset16S.h - Character set for 16 segment MVFD
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

#ifndef CHARSET16S_H
#define CHARSET16S_H

#include "settings.h"

const PROGMEM uint8_t SCROL_FRAMES[3][14] = 
    {
        {
        0x01, 0x10, //bits 8, 4
        0x00, 0x00, //bits -
        0x00, 0x00, //bits -
        0x01, 0x10, //bits 8, 4
        0x00, 0x00, //bits -
        0x00, 0x00, //bits -
        0x01, 0x10, //bits 8, 4
        },
        {
        0x00, 0x01, //bits 0
        0x01, 0x00, //bits 8
        0x00, 0x10, //bits 4
        0x00, 0x00, //bits -
        0x01, 0x00, //bits 8
        0x00, 0x10, //bits 4
        0x20, 0x00, //bits 13
        },
        {
        0x10, 0x00, //bits 12
        0x00, 0x10, //bits 4
        0x01, 0x00, //bits 8
        0x00, 0x00, //bits -
        0x00, 0x10, //bits 4
        0x01, 0x00, //bits 8
        0x00, 0x02, //bits 1
        }
    };

const PROGMEM uint8_t ROTOR_PGM[4][2] = {
    {0x02,0x40}, // | (bits 9, 6)
    {0x04,0x48}, // \ (bits 10, 6, 3)
    {0xC0,0x40}, // - (bits 15, 14, 6)
    {0x08,0x44}  // / (bits 11, 6, 2)
};

const PROGMEM uint8_t FONT_PGM[63][2] = {
    {0x00,0x00}, // space 0x20
    {0xC0,0x60}, // ! -: (bits 15, 14, 6, 5)
    {0x00,0x00}, // " N/A
    {0x00,0x00}, // # N/A
    {0xE3,0x51}, // $ (bits 15, 14, 13, 9, 8, 6, 4, 0)
    {0x00,0x00}, // % N/A
    {0x00,0x00}, // & N/A
    {0x00,0x00}, // ' N/A
    {0x04,0x04}, // ( (bits 10, 2)
    {0x08,0x08}, // ) (bits 11, 3)
    {0xCE,0x4C}, // * (bits 15, 14, 11, 10, 9, 6, 3, 2)
    {0xC2,0x40}, // + (bits 15, 14, 9, 6)
    {0x00,0x00}, // , N/A
    {0xC0,0x40}, // - (bits 15, 14, 6)
    {0x00,0x40}, // . (bit 6)
    {0x08,0x44}, // / (bits 11, 6, 2)
    {0x39,0x57}, // 0 (bits 13, 12, 11, 8, 6, 4, 2, 1, 0)
    {0x20,0x6},  // 1 (bits 13, 2, 1)
    {0x89,0x52}, // 2 (bits 15, 11, 8, 6, 4, 1)
    {0x05,0x54}, // 3 (bits 10, 8, 6, 4, 2)
    {0xC2,0x41}, // 4 (bits 15, 14, 9, 6, 0)
    {0xE1,0x51}, // 5 (bits 15, 14, 13, 8, 6, 4, 0)
    {0xF1,0x51}, // 6 (bits bits 15, 14, 13, 12, 8, 6, 4, 0)
    {0x08,0x54}, // 7 (bits 11, 6, 4, 2)
    {0xF1,0x53}, // 8 (bits 15, 14, 13, 12, 8, 6, 4, 1, 0)
    {0xE1,0x53}, // 9 (bits 15, 14, 13, 8, 6, 4, 1, 0)
    {0x00,0x20}, // : (bit 5)
    {0x00,0x00}, // ; N/A
    {0x04,0x04}, // < (bits 10, 2)
    {0x00,0x00}, // = N/A
    {0x08,0x08}, // > (bits 11, 3)
    {0x00,0x00}, // ? N/A
    {0x00,0x00}, // @ N/A
    {0xF0,0x53}, // A (bits 15, 14, 13, 12, 6, 4, 1, 0)
    {0xA3,0x52}, // B (bits 15, 13, 9, 8, 6, 4, 1 )
    {0x11,0x11}, // C (bits 12, 8, 4, 0)
    {0x23,0x52}, // D (bits 13, 9, 8, 6, 4, 1 )
    {0xD1,0x51}, // E (bits 15, 14, 12, 8, 6, 4, 0)
    {0x50,0x51}, // F (bits 14, 12, 6, 4, 0)
    {0xB1,0x11}, // G (bits 15, 13, 12, 8, 4, 0)
    {0xF0,0x43}, // H (bits 15, 14, 13, 12, 6, 1, 0)
    {0x02,0x40}, // I (bits 9, 6)
    {0x31,0x02}, // J (bits 13, 12, 8, 1)
    {0x54,0x45}, // K (bits 14, 12, 10, 6, 2, 0)
    {0x11,0x01}, // L (bits 12, 8, 0)
    {0x30,0x4F}, // M (bits 13, 12, 6, 3, 2, 1, 0)
    {0x34,0x4B}, // N (bits 13, 12, 10, 6, 3, 1, 0)
    {0x31,0x13}, // O (bits 13, 12, 8, 4, 1, 0)
    {0xD0,0x53}, // P (bits 15, 14, 12, 6, 4, 1, 0)
    {0x35,0x13}, // Q (bits 13, 12, 10, 8, 4, 1, 0)
    {0xD4,0x53}, // R (bits 15, 14, 12, 10, 6, 4, 1, 0)
    {0xE1,0x51}, // S (bits 15, 14, 13, 8, 6, 4, 0)
    {0x02,0x50}, // T (bits 9, 6, 4)
    {0x31,0x03}, // U (bits 13, 12, 8, 1, 0)
    {0x18,0x45}, // V (bits 12, 11, 6, 2, 0)
    {0x3C,0x03}, // W (bits 13, 12, 11, 10, 1, 0)
    {0x0C,0x4C}, // X (bits 11, 10, 6, 3, 2)
    {0xE1,0x43}, // Y (bits 15, 14, 13, 8, 6, 1, 0)
    {0x09,0x54}, // Z (bits 11, 8, 6, 4, 2)
    {0x04,0x04}, // [ (bits 10, 2)
    {0x04,0x48}, // \ (bits 10, 6, 3)
    {0x08,0x08}, // ] (bits 11, 3)
    
    //Special non-standard symbols
    {0xC0, 0x53}  // degree (bits 15, 14, 6, 4, 1, 0)
}; // DEL


#endif

