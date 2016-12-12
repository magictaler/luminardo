/*
  Magictale VFD panel 16 Segments 8 Digits (MVFD-16S8D)
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

#ifndef MVFD_16S8D_H
#define MVFD_16S8D_H

#include "Settings.h"
#include "HT16515.h"
#include "IRRemote.h"
#include "Charset16S.h"
#include "ToneEx.h"

#define VFD_SEGMENTS 16
#define VFD_DIGITS 8
#define VFD_BYTES_PER_DIGIT 3
#define VFD_MAX_TEST_DELAY 100
#define VFD_FLASHING_DELAY 2
#define VFD_ROTOR_TEST_REVOLUTIONS 10
#define VFD_ROTOR_FRAMES 4
#define VFD_SCROLL_FRAMES 3
#define VFD_FLASH_NONE 0
#define VFD_FLASH_ALL 0xFF
#define VFD_FIRST_DISP_CHAR 0x20
#define VFD_FLASH_MODE_BIT 15
#define VFD_ACC_PIN_DEF 255
#define VFD_DEF_TONE 2960

class MVFD_16S8D : public Print 
{
    public:
        enum enum_TestState 
        {   
            NOT_STARTED,    
            COLUMN_TEST,
            SEGMENT_TEST,
            DIMMING_TEST,
            GLOWING_TEST,
            CHARSET_TEST,
            CHARSET_TEST2,
            ROTOR_TEST,
            SLASH_EFFECT,
            SCROLL_EFFECT,
            PRODUCT_NAME,
            COMPLETED    
        };

        MVFD_16S8D(uint8_t cs, uint8_t sclk, uint8_t data, uint8_t standby, uint8_t powerdown);

        void reset();
        void standby(uint8_t isEnabled);
        void powerdown(uint8_t isEnabled);
        void initLED(uint8_t rled, uint8_t gled, uint8_t bled);
        void setLED(uint8_t rval, uint8_t gval, uint8_t bval);
        void setR_LED(uint8_t rval);
        void setG_LED(uint8_t gval);
        void setB_LED(uint8_t bval);
        void initTone(uint8_t tonePin);
        void playTone(uint16_t freq, unsigned long duration);
        void playNoTone();
        void playRtttl(char *p);
        void initLightSensor(uint8_t lsnr);
        uint16_t getLightSensorVal();
        decode_results* getIR_RC_Code();
        void resumeIR_RC();
        void initIR_RC(uint8_t ir, uint8_t spkr);
        void displayOnCmd(uint8_t dimming);
        void displayOffCmd();
        void setCur(uint8_t col);

        //Send ASCII value to display
        size_t write(uint8_t value);
        //Send ASCII value to intermediate buffer
        void write_f(uint8_t value, uint8_t dstIndex, uint8_t dispColumn);

        //Transfer array to display
        uint8_t write(uint8_t* buffer, uint8_t dstIndex, uint8_t len);
        //Transfer array to intermediate buffer
        uint8_t write_f(uint8_t* buffer, uint8_t dstIndex, uint8_t len);
        //Transfer string from program memory to intermediate buffer
        void print_f_p(const char str[]);
        //Transfer whole intermediate buffer to display
        void flipFrame();
        void flipFlashState();
	void clearFrame();

        uint8_t getFlashAttr(uint8_t index);
        void setFlashAttr(uint8_t index, uint8_t value);

        uint8_t columnTest();
        uint8_t segmentTest();
        uint8_t dimmingTest();
        uint8_t glowingTest();
        uint8_t charsetTest();
        uint8_t charsetTest2();
        uint8_t rotorTest();
        inline void renderRotor(uint8_t addr, uint8_t idx);
        uint8_t dispProductName();

        uint8_t slashEffect();
        uint8_t scrollEffect();

        uint8_t testStep();

        void initScroll_p(const char str[]);
        uint8_t scrollStep();

        void spkrOn(uint16_t toggleCnt);

protected: 
        uint8_t _rotorState, _rled, _gled, _bled, _lsnr, _standby, _powerdown;
        uint16_t _flashAttr,_testBt;
        enum_TestState _tstState;
        uint8_t _vfdFrame[VFD_BYTES_PER_DIGIT * VFD_DIGITS];
//        prog_char *scrollPntr;
        HT16515 driver;
        const char *_scrollPntr;
        uint8_t _scrollLeadingIdx;
};

#endif
