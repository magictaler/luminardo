/*
  Magictale VFD panel 16 Segments 8 Digits
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

#include "MVFDPanel_16S8D.h"

IRrecv irrecv;
decode_results IR_results;

MVFD_16S8D::MVFD_16S8D(uint8_t cs, uint8_t sclk, uint8_t data, uint8_t standby, uint8_t powerdown)
{
    driver.init(cs, sclk, data);
    _standby = standby;
    _powerdown = powerdown;
    _rled = VFD_ACC_PIN_DEF;
    _gled = VFD_ACC_PIN_DEF;
    _bled = VFD_ACC_PIN_DEF;
    _lsnr = VFD_ACC_PIN_DEF;
//    irparams.recvpin = VFD_ACC_PIN_DEF;

    pinMode(_standby, OUTPUT);
    digitalWrite(_standby, HIGH);

    pinMode(_powerdown, OUTPUT);
    digitalWrite(_powerdown, HIGH);
}

void MVFD_16S8D::initLED(uint8_t rled, uint8_t gled, uint8_t bled)
{
    _rled = rled;
    _gled = gled;
    _bled = bled;

    pinMode(_rled, OUTPUT);
    analogWrite(_rled, 0);

    pinMode(_gled, OUTPUT);
    analogWrite(_gled, 0);

    pinMode(_bled, OUTPUT);
    analogWrite(_bled, 0);
}

void MVFD_16S8D::setLED(uint8_t rval, uint8_t gval, uint8_t bval)
{
    analogWrite(_rled, rval);
    analogWrite(_gled, gval);
    analogWrite(_bled, bval);
}

void MVFD_16S8D::setR_LED(uint8_t rval)
{
    analogWrite(_rled, rval);
}

void MVFD_16S8D::setG_LED(uint8_t gval)
{
    analogWrite(_gled, gval);
}

void MVFD_16S8D::setB_LED(uint8_t bval)
{
    analogWrite(_bled, bval);
}

void MVFD_16S8D::initTone(uint8_t tonepin)
{
    Tone.init(tonepin);
}

void MVFD_16S8D::playTone(uint16_t freq, unsigned long duration)
{
    Tone.tone(freq, duration);
}

void MVFD_16S8D::playNoTone()
{  
    Tone.noTone();
}

void MVFD_16S8D::playRtttl(char *p)
{
    Tone.playRtttl(p);
}

void MVFD_16S8D::initLightSensor(uint8_t lsnr)
{
    _lsnr = lsnr;
    pinMode(_lsnr, INPUT);
}

uint16_t MVFD_16S8D::getLightSensorVal()
{
    return analogRead(_lsnr);
}
 
void MVFD_16S8D::initIR_RC(uint8_t ir)
{
    irrecv.enableIRIn(ir);
}

decode_results* MVFD_16S8D::getIR_RC_Code()
{
  if (irrecv.decode(&IR_results)) 
    return &IR_results;
  else 
    return NULL;
}

void MVFD_16S8D::resumeIR_RC()
{
  irrecv.resume();
}

void MVFD_16S8D::standby(uint8_t isEnabled)
{
    if (isEnabled)
        digitalWrite(_standby, HIGH);
    else
        digitalWrite(_standby, LOW);
}

void MVFD_16S8D::powerdown(uint8_t isEnabled)
{
    if (isEnabled)
        digitalWrite(_powerdown, HIGH);
    else
        digitalWrite(_powerdown, LOW);
}

void MVFD_16S8D::reset()
{
    driver.reset(VFD_DISP_MODE_12D16S);
    driver.addrSetCmd(0);
    clearFrame();
    flipFrame();

    _testBt = 0;
    _tstState = NOT_STARTED;

    _flashAttr = 0;
}

void MVFD_16S8D::displayOnCmd(uint8_t dimming)
{
    driver.displayOnCmd(dimming);
}

void MVFD_16S8D::displayOffCmd()
{
    driver.displayOffCmd();
}

void MVFD_16S8D::setCur(uint8_t col)
{
    if (col >= VFD_DIGITS) col = 0;
    driver.addrSetCmd(col * VFD_BYTES_PER_DIGIT);
}

size_t MVFD_16S8D::write(uint8_t value) 
{
    driver.displayMemWriteCmd(true, false);

    if (value < VFD_FIRST_DISP_CHAR) value = VFD_FIRST_DISP_CHAR;

    for (uint8_t i = 0; i < VFD_BYTES_PER_DIGIT; i++)
    {
        uint8_t chrset = pgm_read_byte(&FONT_PGM[(value - VFD_FIRST_DISP_CHAR)][i]);
        driver.data(chrset, false, ((i + 1) == VFD_BYTES_PER_DIGIT));
    }
    return 1;
}

void MVFD_16S8D::write_f(uint8_t value, uint8_t dstIndex, uint8_t dispColumn) 
{
    if (dstIndex > (VFD_DIGITS - 1) * VFD_BYTES_PER_DIGIT) return;

    if (value < VFD_FIRST_DISP_CHAR) value = VFD_FIRST_DISP_CHAR;

    for (uint8_t i = 0; i < (VFD_BYTES_PER_DIGIT); i++)
    {
        uint8_t chrset = pgm_read_byte(&FONT_PGM[(value - VFD_FIRST_DISP_CHAR)][i]);

        //Fixme later: if (dispColumn) chrset |= 1 << 5;
        if (i == 1 && dispColumn) chrset |= 1 << 5;

        _vfdFrame[dstIndex + i] = chrset;
    }
}

uint8_t MVFD_16S8D::write(uint8_t* buffer, uint8_t dstIndex, uint8_t len)
{
    if (len == 0 || (dstIndex >= VFD_DIGITS * VFD_BYTES_PER_DIGIT)) return 0;
    if (len > VFD_DIGITS * VFD_BYTES_PER_DIGIT - dstIndex) 
        len = VFD_DIGITS * VFD_BYTES_PER_DIGIT - dstIndex;

    driver.addrSetCmd(dstIndex);

    driver.displayMemWriteCmd(true, false);
    for (uint8_t i = 0; i < len-1; i++)
    {
         driver.data(buffer[i], false, false);
    } 
    driver.data(buffer[len-1], false, true);

    return len;
}

uint8_t MVFD_16S8D::write_f(uint8_t* buffer, uint8_t dstIndex, uint8_t len)
{
    if (len == 0 || (dstIndex >= VFD_DIGITS * VFD_BYTES_PER_DIGIT)) return 0;
    if (len > VFD_DIGITS * VFD_BYTES_PER_DIGIT - dstIndex) 
        len = VFD_DIGITS * VFD_BYTES_PER_DIGIT - dstIndex;

    memcpy(&_vfdFrame[dstIndex], buffer, len);

    return len;
}

void MVFD_16S8D::print_f_p(const prog_char str[])
{
    char c;
    uint8_t idx = 0;
    if(!str) return;

    clearFrame();
   
    while((c = pgm_read_byte(str++)))
    {
        write_f(c, idx, false);
        idx += VFD_BYTES_PER_DIGIT;
    }
}

uint8_t MVFD_16S8D::testStep()
{
    switch (_tstState)
    {
        case NOT_STARTED:
            {
	        _tstState = COLUMN_TEST;
                _testBt = 0;
            }
            break;

        case COLUMN_TEST:
            if (!columnTest()) 
            {
                _testBt = 0;
                _tstState = SEGMENT_TEST;
            }
            break;

        case SEGMENT_TEST:
            if (!segmentTest()) 
            {
                _tstState = DIMMING_TEST;
                _testBt = VFD_DIMMING_MAX;
            }
            break;

        case DIMMING_TEST:
            if (!dimmingTest())
            {
                _tstState = GLOWING_TEST;
                _testBt = 0;
            }
            break;

        case GLOWING_TEST:
            if (!glowingTest())
            {
                _tstState = CHARSET_TEST;
                _testBt = (VFD_MAX_TEST_DELAY / 4);
            }
            break;

        case CHARSET_TEST:
            if (!charsetTest())
            {
                _tstState = CHARSET_TEST2;
                _testBt = VFD_MAX_TEST_DELAY / 2;
            }
            break;

        case CHARSET_TEST2:
            if (!charsetTest2())
            {
               _tstState = ROTOR_TEST;
               _rotorState = 0;
               _testBt = VFD_ROTOR_FRAMES * VFD_ROTOR_TEST_REVOLUTIONS;
            }
            break;

        case ROTOR_TEST:
            if (!rotorTest())
            {
                _tstState = SCROLL_EFFECT;
                _testBt = (VFD_MAX_TEST_DELAY / 2);
                _rotorState = 0;
            }
            break;

        case SCROLL_EFFECT:
            //if (!scrollEffect())
            {
                //_tstState = COMPLETED;
                _tstState = PRODUCT_NAME;
                _testBt = (VFD_MAX_TEST_DELAY / 4);
            }
            break;

        case SLASH_EFFECT:
            if (!slashEffect())
                _tstState = COMPLETED;
            break;
 
        case PRODUCT_NAME:
            if (!dispProductName())
                _tstState = COMPLETED;
            break;

        case COMPLETED:
            _tstState = NOT_STARTED;
            break;
        default:
            _tstState = NOT_STARTED;
    }
    return _tstState;
}

uint8_t MVFD_16S8D::columnTest()
{
    if (_testBt < VFD_DIGITS)
    {
        if (_testBt == 0)
        {
            driver.addrSetCmd(0);
            for (uint8_t i = 0; i < VFD_DIGITS; i++)
            {
                driver.displayMemWriteCmd(true, false);
                for (uint8_t j = 0; j < VFD_BYTES_PER_DIGIT; j++)
                {
                    if (i == 0) 
                    {
                        driver.data(0xFF, false, ((j + 1) == VFD_BYTES_PER_DIGIT));
                    }else{
                        driver.data(0, false, ((j + 1) == VFD_BYTES_PER_DIGIT));
                    }
                }
            }
        }else{
            driver.addrSetCmd((_testBt - 1) * VFD_BYTES_PER_DIGIT);
            driver.displayMemWriteCmd(true, false);
            for (uint8_t j = 0; j < (VFD_BYTES_PER_DIGIT); j++)
                driver.data(0, false, false);
            for (uint8_t j = 0; j < (VFD_BYTES_PER_DIGIT); j++)
                driver.data(0xFF, false, ((j + 1) == VFD_BYTES_PER_DIGIT));
        }
        _testBt++;
        return true;
    }else return false;
}

uint8_t MVFD_16S8D::segmentTest()
{
    if (_testBt < VFD_SEGMENTS)
    {
        driver.addrSetCmd(0);
        for (uint8_t i = 0; i < VFD_DIGITS; i++)
        {
            driver.displayMemWriteCmd(true, false);
            driver.data((1 << _testBt ) & 0xFF, false, false);
            driver.data(((1 << _testBt ) >> 8) & 0xFF, false, false);
            driver.data(((1 << _testBt ) >> 16) & 0xFF, false, true);
        }
        _testBt++;
        return true;
    }else return false;
}

uint8_t MVFD_16S8D::dimmingTest()
{
    if (_testBt == VFD_DIMMING_MAX)
    {
        driver.addrSetCmd(0);
        for (uint8_t i = 0; i < VFD_DIGITS; i++)
        {
            driver.displayMemWriteCmd(true, false);
            for (uint8_t j = 0; j < VFD_BYTES_PER_DIGIT; j++)
                driver.data(0xFF, false, ((j + 1) == VFD_BYTES_PER_DIGIT));
        }

        displayOnCmd(_testBt);
        _testBt--;
        return true;
    }else
    {
        displayOnCmd(_testBt);
        if (_testBt != 0)
        {
            _testBt--;
            return true;
        }
    }
    return false;
}

uint8_t MVFD_16S8D::glowingTest()
{
    if (_testBt == 0)
    {
        driver.addrSetCmd(0);
        for (uint8_t i = 0; i < VFD_DIGITS; i++)
        {
            driver.displayMemWriteCmd(true, false);
            for (uint8_t j = 0; j < VFD_BYTES_PER_DIGIT; j++)
                driver.data(0xFF, false, ((j + 1) == VFD_BYTES_PER_DIGIT));
            driver.data(0xFF, false, false);
            driver.data(0xFF, false, true);
        }

        displayOnCmd(_testBt);
        _testBt++;
        return true;
    }else
    {
        displayOnCmd(_testBt);
        if (_testBt < VFD_DIMMING_MAX)
        {
            _testBt++;
            return true;
        }
    }
    return false;
}


uint8_t MVFD_16S8D::charsetTest2()
{
    if (_testBt != 0)
    {
        if (_testBt == (VFD_MAX_TEST_DELAY / 2))
        {
            //Sending characters one by one

            //addrSetCmd(0);
            //for (uint8_t testChar = 0x30; testChar < (0x30 + VFD_DIGITS); testChar++)
            //    write(testChar);
    
            //Sending characters to intermediate buffer and
            //then send video data in one go
            //for (uint8_t testChar = 0x57; testChar < (0x57 + VFD_DIGITS); testChar++)
            //    write_f(testChar, ( testChar - 0x57 ) * VFD_BYTES_PER_DIGIT, false);
            driver.addrSetCmd(0);
            print_f_p(PSTR("()*+-./\\"));
            
            _flashAttr = 0;
            flipFrame();

            setFlashAttr(VFD_DIGITS + 1, (1 << (VFD_DIGITS - 1)));
        }else
        {
            //Using flashing attributes
            if (_testBt % VFD_FLASHING_DELAY == 0)
            {
                flipFlashState();
                flipFrame();
            }
            if ((_testBt % (VFD_FLASHING_DELAY * 4)) == 0)
            {
                uint8_t flashVal = (getFlashAttr(VFD_DIGITS + 1) >> 1);
                if (flashVal == 0) flashVal = (1 << (VFD_DIGITS - 1));
                setFlashAttr(VFD_DIGITS + 1, flashVal);
            }
        }
        _testBt--;
        return true;
    }
    return false;
}

uint8_t MVFD_16S8D::charsetTest()
{
    if (_testBt != 0)
    {
        if (_testBt == VFD_MAX_TEST_DELAY / 4)
        {
            driver.addrSetCmd(0);
            for (uint8_t testChar = 0x30; testChar < 0x30 + VFD_DIGITS; testChar++)
                write(testChar);
        }else if (_testBt == VFD_MAX_TEST_DELAY * 3 / 16)
        {
            driver.addrSetCmd(0);
            print(F("ABCDEFGH"));
        }else if (_testBt == VFD_MAX_TEST_DELAY / 8)
        {
            driver.addrSetCmd(0);
            print(F("IJKLMNOP"));
        }else if (_testBt == VFD_MAX_TEST_DELAY / 16)
        {
            driver.addrSetCmd(0);
            print(F("QRSTUVWX"));
        }
        _testBt--;
        return true;
    }
    return false;
}

uint8_t MVFD_16S8D::dispProductName()
{
    if (_testBt != 0)
    {
        if (_testBt == VFD_MAX_TEST_DELAY * 5 / 20)
        {
            driver.addrSetCmd(0);
            print(F(" MAGIC  "));
        }else if (_testBt == VFD_MAX_TEST_DELAY * 4 / 20)
        {
            driver.addrSetCmd(0);
            print(F("  VFD   "));            
        }else if (_testBt == VFD_MAX_TEST_DELAY * 3 / 20)
        {
            driver.addrSetCmd(0);
            print(F(" PANEL  "));
        }else if (_testBt == VFD_MAX_TEST_DELAY * 2 / 20)
        {
            driver.addrSetCmd(0);
            print(F("16 SEGMN"));
        }else if (_testBt == VFD_MAX_TEST_DELAY * 1 / 20)
        {
            driver.addrSetCmd(0);
            print(F("8 DIGITS"));
        }
        _testBt--;
        return true;
    }
    return false;
}

inline void MVFD_16S8D::renderRotor(uint8_t addr, uint8_t idx)
{
    driver.addrSetCmd(addr);
    driver.displayMemWriteCmd(true, false);
    uint8_t chrset = pgm_read_byte(&ROTOR_PGM[idx][0]);
    driver.data(chrset, false, false);
    chrset = pgm_read_byte(&ROTOR_PGM[idx][1]);
    driver.data(chrset, false, true);
}

uint8_t MVFD_16S8D::rotorTest()
{
    if (_testBt!= 0)
    {
        if (_testBt == VFD_ROTOR_FRAMES * VFD_ROTOR_TEST_REVOLUTIONS)
        {
            driver.addrSetCmd(0);
            for (uint8_t i = 0; i < VFD_DIGITS; i++)
            {
                driver.displayMemWriteCmd(true, false);
                driver.data(0, false, false);
                driver.data(0, false, false);
                driver.data(0, false, true);
            }
        }

        renderRotor(0, (_rotorState & 0xF));

        uint8_t bckRotSt = VFD_ROTOR_FRAMES - ((_rotorState >> 4) & 0xF);
        if (bckRotSt == VFD_ROTOR_FRAMES) bckRotSt = 0;
        renderRotor(2 * VFD_BYTES_PER_DIGIT, bckRotSt);

        renderRotor(5 * VFD_BYTES_PER_DIGIT, ((_rotorState >> 4) & 0xF));

        bckRotSt = VFD_ROTOR_FRAMES - (_rotorState & 0xF);
        if (bckRotSt == VFD_ROTOR_FRAMES) bckRotSt = 0;
        renderRotor((VFD_DIGITS - 1) * VFD_BYTES_PER_DIGIT, bckRotSt);

        _rotorState = (_rotorState & 0xF0) | ((_rotorState & 0xF) + 1);
        if ((_rotorState & 0xF) >= VFD_ROTOR_FRAMES) _rotorState &= 0xF0;

        if (_testBt % 2 == 0)
        {
            _rotorState = ((_rotorState + 0x10) & 0xF0) | (_rotorState & 0xF);
            if ((( _rotorState >> 4 ) & 0xF) >= VFD_ROTOR_FRAMES) _rotorState &= 0xF;
        }

        _testBt--;
        return true;
    }   
    return false;
}

void MVFD_16S8D::flipFrame()
{
    if (_flashAttr == 0 || ((_flashAttr >> VFD_FLASH_MODE_BIT ) & 0x1) == 0)
    {
        write((uint8_t*)&_vfdFrame, 0, VFD_BYTES_PER_DIGIT * VFD_DIGITS);
        return;
    }

    for (uint8_t i = 0; i < VFD_DIGITS; i++)
    {
        if ((( _flashAttr >> (VFD_DIGITS - i - 1) ) & 0x1) == 0)
        {
            //Send bytes representing single digit to VFD
            write(&_vfdFrame[i * VFD_BYTES_PER_DIGIT], i * VFD_BYTES_PER_DIGIT, 
                VFD_BYTES_PER_DIGIT);
        }else{
            //The digit is not visible during this period in time
            driver.addrSetCmd(i * VFD_BYTES_PER_DIGIT);
            //Display space char
            write(0x20);    
        }
    }
}

void MVFD_16S8D::clearFrame()
{
    memset(&_vfdFrame, 0, sizeof(_vfdFrame));
}

uint8_t MVFD_16S8D::getFlashAttr(uint8_t index)
{
    if (index >= VFD_DIGITS) return (_flashAttr & ~(1 << VFD_FLASH_MODE_BIT));
    else return (( _flashAttr >> index ) & 0x1);
}
     
void MVFD_16S8D::setFlashAttr(uint8_t index, uint8_t value)
{
    if (index >= VFD_DIGITS) 
        _flashAttr = (_flashAttr & (1 << VFD_FLASH_MODE_BIT)) | (value & ~(1 << VFD_FLASH_MODE_BIT));
    else _flashAttr |= 1 << index;
}

void MVFD_16S8D::flipFlashState()
{
    if (((_flashAttr >> VFD_FLASH_MODE_BIT ) & 0x1) == 0)
    {
        _flashAttr |= 1 << VFD_FLASH_MODE_BIT;
    }else{
        _flashAttr &= ~(1 << VFD_FLASH_MODE_BIT);
    }
}

uint8_t MVFD_16S8D::slashEffect()
{
    if (_testBt != 0)
    {
        if (_testBt == VFD_DIGITS)
        {
            memset(&_vfdFrame, 0, sizeof(_vfdFrame));
            write_f(0x2F, 0, false);
            write_f(0x5C, (VFD_DIGITS - 1) * VFD_BYTES_PER_DIGIT, false );
        }else{
            //Move everything to the center
            uint16_t* vfdPntr = (uint16_t*)&_vfdFrame;
            uint16_t leftVal, rightVal;
            for (uint8_t i = ((VFD_DIGITS - 1) / 2 + 1); i < VFD_DIGITS; i++)
            {
                leftVal = vfdPntr[VFD_DIGITS - 1 - i];
                rightVal = vfdPntr[i];
                
                vfdPntr[i - 1] |= rightVal;
                vfdPntr[VFD_DIGITS - i] |= leftVal;
            }
            write_f(0x2F, 0, false);
            write_f(0x5C, (VFD_DIGITS - 1) * VFD_BYTES_PER_DIGIT, false);
        }

        flipFrame();

        _testBt--;
        return true;
    }   
    return false;
}

uint8_t MVFD_16S8D::scrollEffect()
{
    if (_testBt != 0)
    {
        for (uint8_t i = 0; i < sizeof(_vfdFrame); i++)
            _vfdFrame[i] = pgm_read_byte(&SCROL_FRAMES[_rotorState][i]);

        _flashAttr = 0;  
        flipFrame();

        _rotorState++;
        if (_rotorState >= VFD_SCROLL_FRAMES) _rotorState = 0;

        _testBt--;
        return true;    
    }
    return false;
}

void MVFD_16S8D::initScroll_p(const prog_char str[])
{
    _scrollPntr = (prog_char*)str;
}

uint8_t MVFD_16S8D::scrollStep( void )
{
    char c;
    uint8_t idx = 0;
    uint8_t digitsLeft = VFD_DIGITS;
    if(!_scrollPntr) return false;

    prog_char* str = _scrollPntr;

    clearFrame();

    //Filling in blank spaces first so that scrolling 
    //starts from the very right
    for(uint8_t i = 0; i < _scrollLeadingIdx; i++)
    {
        write_f(0x20, idx, false);
        idx += VFD_BYTES_PER_DIGIT;
        digitsLeft--;
    }
   
    while((digitsLeft > 0) && (c = pgm_read_byte(str++)))
    {
        write_f(c, idx, false);
        idx += VFD_BYTES_PER_DIGIT;
        digitsLeft--;
    }

    flipFrame();

    if (_scrollLeadingIdx > 0)
    {
        _scrollLeadingIdx--;
        return true;
    }
    else _scrollPntr++;

    if (!pgm_read_byte(_scrollPntr)) return false;
    else return true;
}

