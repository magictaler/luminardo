/* ToneEx.cpp

  A mixture of Tone Generator library by Brett Hagman, D Mellis, M Sproul 
  with RTTTL (RingTone Text Transfer Language) songs from
  http://code.google.com/p/rogue-code/wiki/ToneLibraryDocumentation

  Compiled into a class and enhanced by Dmitry Pakhomenko
  dmitryp@magictale.com
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

#include "ToneEx.h"

#include <util/delay.h>

ToneEx::ToneEx()
{


}

void ToneEx::init(uint8_t pin)
{
  _pin = pin;
}

int8_t ToneEx::toneBegin()
{
  int8_t _timer = -1;

  // if we're already using the pin, the timer should be configured.  
  for (int i = 0; i < AVAILABLE_TONE_PINS; i++) {
    if (tone_pins[i] == _pin) {
      return pgm_read_byte(tone_pin_to_timer_PGM + i);
    }
  }
  
  // search for an unused timer.
  for (int i = 0; i < AVAILABLE_TONE_PINS; i++) {
    if (tone_pins[i] == 255) {
      tone_pins[i] = _pin;
      _timer = pgm_read_byte(tone_pin_to_timer_PGM + i);
      break;
    }
  }
  
  if (_timer != -1)
  {
    // Set timer specific stuff
    // All timers in CTC mode
    // 8 bit timers will require changing prescalar values,
    // whereas 16 bit timers are set to either ck/1 or ck/64 prescalar
    switch (_timer)
    {
      #if defined(TCCR0A) && defined(TCCR0B)
      case 0:
        // 8 bit timer
        TCCR0A = 0;
        TCCR0B = 0;
        bitWrite(TCCR0A, WGM01, 1);
        bitWrite(TCCR0B, CS00, 1);
        timer0_pin_port = portOutputRegister(digitalPinToPort(_pin));
        timer0_pin_mask = digitalPinToBitMask(_pin);
        break;
      #endif

      #if defined(TCCR1A) && defined(TCCR1B) && defined(WGM12)
      case 1:
        // 16 bit timer
        TCCR1A = 0;
        TCCR1B = 0;
        bitWrite(TCCR1B, WGM12, 1);
        bitWrite(TCCR1B, CS10, 1);
        timer1_pin_port = portOutputRegister(digitalPinToPort(_pin));
        timer1_pin_mask = digitalPinToBitMask(_pin);
        break;
      #endif

      #if defined(TCCR2A) && defined(TCCR2B)
      case 2:
        // 8 bit timer
        TCCR2A = 0;
        TCCR2B = 0;
        bitWrite(TCCR2A, WGM21, 1);
        bitWrite(TCCR2B, CS20, 1);
        timer2_pin_port = portOutputRegister(digitalPinToPort(_pin));
        timer2_pin_mask = digitalPinToBitMask(_pin);
        break;
      #endif

      #if defined(TCCR3A) && defined(TCCR3B) &&  defined(TIMSK3)
      case 3:
        // 16 bit timer
        TCCR3A = 0;
        TCCR3B = 0;
        bitWrite(TCCR3B, WGM32, 1);
        bitWrite(TCCR3B, CS30, 1);
        timer3_pin_port = portOutputRegister(digitalPinToPort(_pin));
        timer3_pin_mask = digitalPinToBitMask(_pin);
        break;
      #endif

      #if defined(TCCR4A) && defined(TCCR4B) &&  defined(TIMSK4)
      case 4:
        // 16 bit timer
        TCCR4A = 0;
        TCCR4B = 0;
        #if defined(WGM42)
          bitWrite(TCCR4B, WGM42, 1);
        #elif defined(CS43)
          #warning this may not be correct
          // atmega32u4
          bitWrite(TCCR4B, CS43, 1);
        #endif
        bitWrite(TCCR4B, CS40, 1);
        timer4_pin_port = portOutputRegister(digitalPinToPort(_pin));
        timer4_pin_mask = digitalPinToBitMask(_pin);
        break;
      #endif

      #if defined(TCCR5A) && defined(TCCR5B) &&  defined(TIMSK5)
      case 5:
        // 16 bit timer
        TCCR5A = 0;
        TCCR5B = 0;
        bitWrite(TCCR5B, WGM52, 1);
        bitWrite(TCCR5B, CS50, 1);
        timer5_pin_port = portOutputRegister(digitalPinToPort(_pin));
        timer5_pin_mask = digitalPinToBitMask(_pin);
        break;
      #endif
    }
  }

  return _timer;
}

// frequency (in hertz) and duration (in milliseconds).
void ToneEx::tone(unsigned int frequency, unsigned long duration)
{
  uint8_t prescalarbits = 0b001;
  long toggle_count = 0;
  uint32_t ocr = 0;
  int8_t _timer;

  _timer = toneBegin();

  if (_timer >= 0)
  {
    // Set the pinMode as OUTPUT
    pinMode(_pin, OUTPUT);
    
    // if we are using an 8 bit timer, scan through prescalars to find the best fit
    if (_timer == 0 || _timer == 2)
    {
      ocr = F_CPU / frequency / 2 - 1;
      prescalarbits = 0b001;  // ck/1: same for both timers
      if (ocr > 255)
      {
        ocr = F_CPU / frequency / 2 / 8 - 1;
        prescalarbits = 0b010;  // ck/8: same for both timers

        if (_timer == 2 && ocr > 255)
        {
          ocr = F_CPU / frequency / 2 / 32 - 1;
          prescalarbits = 0b011;
        }

        if (ocr > 255)
        {
          ocr = F_CPU / frequency / 2 / 64 - 1;
          prescalarbits = _timer == 0 ? 0b011 : 0b100;

          if (_timer == 2 && ocr > 255)
          {
            ocr = F_CPU / frequency / 2 / 128 - 1;
            prescalarbits = 0b101;
          }

          if (ocr > 255)
          {
            ocr = F_CPU / frequency / 2 / 256 - 1;
            prescalarbits = _timer == 0 ? 0b100 : 0b110;
            if (ocr > 255)
            {
              // can't do any better than /1024
              ocr = F_CPU / frequency / 2 / 1024 - 1;
              prescalarbits = _timer == 0 ? 0b101 : 0b111;
            }
          }
        }
      }

#if defined(TCCR0B)
      if (_timer == 0)
      {
        TCCR0B = prescalarbits;
      }
      else
#endif
#if defined(TCCR2B)
      {
        TCCR2B = prescalarbits;
      }
#else
      {
        // dummy place holder to make the above ifdefs work
      }
#endif
    }
    else
    {
      // two choices for the 16 bit timers: ck/1 or ck/64
      ocr = F_CPU / frequency / 2 - 1;

      prescalarbits = 0b001;
      if (ocr > 0xffff)
      {
        ocr = F_CPU / frequency / 2 / 64 - 1;
        prescalarbits = 0b011;
      }

      if (_timer == 1)
      {
#if defined(TCCR1B)
        TCCR1B = (TCCR1B & 0b11111000) | prescalarbits;
#endif
      }
#if defined(TCCR3B)
      else if (_timer == 3)
        TCCR3B = (TCCR3B & 0b11111000) | prescalarbits;
#endif
#if defined(TCCR4B)
      else if (_timer == 4)
        TCCR4B = (TCCR4B & 0b11111000) | prescalarbits;
#endif
#if defined(TCCR5B)
      else if (_timer == 5)
        TCCR5B = (TCCR5B & 0b11111000) | prescalarbits;
#endif

    }
    

    // Calculate the toggle count
    if (duration > 0)
    {
      toggle_count = 2 * frequency * duration / 1000;
    }
    else
    {
      toggle_count = -1;
    }

    // Set the OCR for the given timer,
    // set the toggle count,
    // then turn on the interrupts
    switch (_timer)
    {

#if defined(OCR0A) && defined(TIMSK0) && defined(OCIE0A)
      case 0:
        OCR0A = ocr;
        timer0_toggle_count = toggle_count;
        bitWrite(TIMSK0, OCIE0A, 1);
        break;
#endif

      case 1:
#if defined(OCR1A) && defined(TIMSK1) && defined(OCIE1A)
        OCR1A = ocr;
        timer1_toggle_count = toggle_count;
        bitWrite(TIMSK1, OCIE1A, 1);
#elif defined(OCR1A) && defined(TIMSK) && defined(OCIE1A)
        // this combination is for at least the ATmega32
        OCR1A = ocr;
        timer1_toggle_count = toggle_count;
        bitWrite(TIMSK, OCIE1A, 1);
#endif
        break;

#if defined(OCR2A) && defined(TIMSK2) && defined(OCIE2A)
      case 2:
        OCR2A = ocr;
        timer2_toggle_count = toggle_count;
        bitWrite(TIMSK2, OCIE2A, 1);
        break;
#endif

#if defined(TIMSK3)
      case 3:
        OCR3A = ocr;
        timer3_toggle_count = toggle_count;
        bitWrite(TIMSK3, OCIE3A, 1);
        break;
#endif

#if defined(TIMSK4)
      case 4:
        OCR4A = ocr;
        timer4_toggle_count = toggle_count;
        bitWrite(TIMSK4, OCIE4A, 1);
        break;
#endif

#if defined(OCR5A) && defined(TIMSK5) && defined(OCIE5A)
      case 5:
        OCR5A = ocr;
        timer5_toggle_count = toggle_count;
        bitWrite(TIMSK5, OCIE5A, 1);
        break;
#endif

    }
  }
}


// XXX: this function only works properly for timer 2 (the only one we use
// currently).  for the others, it should end the tone, but won't restore
// proper PWM functionality for the timer.
void ToneEx::disableTimer(uint8_t _timer)
{
  switch (_timer)
  {
    case 0:
      #if defined(TIMSK0)
        TIMSK0 = 0;
      #elif defined(TIMSK)
        TIMSK = 0; // atmega32
      #endif
      break;

#if defined(TIMSK1) && defined(OCIE1A)
    case 1:
      bitWrite(TIMSK1, OCIE1A, 0);
      break;
#endif

    case 2:
      #if defined(TIMSK2) && defined(OCIE2A)
        bitWrite(TIMSK2, OCIE2A, 0); // disable interrupt
      #endif
      #if defined(TCCR2A) && defined(WGM20)
        TCCR2A = (1 << WGM20);
      #endif
      #if defined(TCCR2B) && defined(CS22)
        TCCR2B = (TCCR2B & 0b11111000) | (1 << CS22);
      #endif
      #if defined(OCR2A)
        OCR2A = 0;
      #endif
      break;

#if defined(TIMSK3)
    case 3:
      TIMSK3 = 0;
      break;
#endif

#if defined(TIMSK4)
    case 4:
      TIMSK4 = 0;
      break;
#endif

#if defined(TIMSK5)
    case 5:
      TIMSK5 = 0;
      break;
#endif
  }
}


void ToneEx::noTone()
{
  int8_t _timer = -1;
  
  for (int i = 0; i < AVAILABLE_TONE_PINS; i++) {
    if (tone_pins[i] == _pin) {
      _timer = pgm_read_byte(tone_pin_to_timer_PGM + i);
      tone_pins[i] = 255;
    }
  }
  
  disableTimer(_timer);

  digitalWrite(_pin, 0);

  _isPlaying = false;
}

#if 0
#if !defined(__AVR_ATmega8__)
ISR(TIMER0_COMPA_vect)
{
  if (timer0_toggle_count != 0)
  {
    // toggle the pin
    *timer0_pin_port ^= timer0_pin_mask;

    if (timer0_toggle_count > 0)
      timer0_toggle_count--;
  }
  else
  {
    disableTimer(0);
    *timer0_pin_port &= ~(timer0_pin_mask);  // keep pin low after stop
  }
}
#endif


ISR(TIMER1_COMPA_vect)
{
  if (timer1_toggle_count != 0)
  {
    // toggle the pin
    *timer1_pin_port ^= timer1_pin_mask;

    if (timer1_toggle_count > 0)
      timer1_toggle_count--;
  }
  else
  {
    disableTimer(1);
    *timer1_pin_port &= ~(timer1_pin_mask);  // keep pin low after stop
  }
}
#endif

ISR(TIMER2_COMPA_vect)
{

  if (timer2_toggle_count != 0)
  {
    // toggle the pin
    *timer2_pin_port ^= timer2_pin_mask;

    if (timer2_toggle_count > 0)
      timer2_toggle_count--;
  }
  else
  {
    // need to call noTone() so that the tone_pins[] entry is reset, so the
    // timer gets initialized next time we call tone().
    // XXX: this assumes timer 2 is always the first one used.

    if (Tone.isPlaying())
    {
      if (!Tone.pushNextNote())
      {
          Tone.abortPlay();
      }
    }else 
      Tone.noTone();
//    disableTimer(2);
//    *timer2_pin_port &= ~(timer2_pin_mask);  // keep pin low after stop
  }
}


//#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)


ISR(TIMER3_COMPA_vect)
{
  if (timer3_toggle_count != 0)
  {
    // toggle the pin
    *timer3_pin_port ^= timer3_pin_mask;

    if (timer3_toggle_count > 0)
      timer3_toggle_count--;
  }
  else
  {
    if (Tone.isPlaying())
    {
      if (!Tone.pushNextNote())
      {
          Tone.abortPlay();
      }
    }else 
      Tone.noTone();

//    disableTimer(3);
//    *timer3_pin_port &= ~(timer3_pin_mask);  // keep pin low after stop
  }
}

#if 0

ISR(TIMER4_COMPA_vect)
{
  if (timer4_toggle_count != 0)
  {
    // toggle the pin
    *timer4_pin_port ^= timer4_pin_mask;

    if (timer4_toggle_count > 0)
      timer4_toggle_count--;
  }
  else
  {
    disableTimer(4);
    *timer4_pin_port &= ~(timer4_pin_mask);  // keep pin low after stop
  }
}

ISR(TIMER5_COMPA_vect)
{
  if (timer5_toggle_count != 0)
  {
    // toggle the pin
    *timer5_pin_port ^= timer5_pin_mask;

    if (timer5_toggle_count > 0)
      timer5_toggle_count--;
  }
  else
  {
    disableTimer(5);
    *timer5_pin_port &= ~(timer5_pin_mask);  // keep pin low after stop
  }
}



#endif

bool ToneEx::isPlaying()
{
  return _isPlaying;
}

void ToneEx::abortPlay()
{
  noTone();
  _isPlaying = false;
}

void ToneEx::playRtttl(char *p)
{
  // Absolutely no error checking in here

  if (isPlaying()) abortPlay();

  _default_dur = 4;
  _default_oct = 6;
  _bpm = 63;

  // format: d=N,o=N,b=NNN:
  // find the start (skip name, etc)

  while(*p != ':') p++;    // ignore name
  p++;                     // skip ':'

  // get default duration
  if(*p == 'd')
  {
    p++; p++;              // skip "d="
    _num = 0;
    while(isdigit(*p))
    {
      _num = (_num * 10) + (*p++ - '0');
    }
    if(_num > 0) _default_dur = _num;
    p++;                   // skip comma
  }

  // get default octave
  if(*p == 'o')
  {
    p++; p++;              // skip "o="
    _num = *p++ - '0';
    if(_num >= 3 && _num <=7) _default_oct = _num;
    p++;                   // skip comma
  }

  // get BPM
  if(*p == 'b')
  {
    p++; p++;              // skip "b="
    _num = 0;
    while(isdigit(*p))
    {
      _num = (_num * 10) + (*p++ - '0');
    }
    _bpm = _num;
    p++;                   // skip colon
  }

  // BPM usually expresses the number of quarter notes per minute
  _wholenote = (60 * 1000L / _bpm) * 4;  // this is the time for whole note (in milliseconds)

  _tunePntr = p;
  _isPlaying = true;

  pushNextNote();
}

bool ToneEx::pushNextNote()
{
  // now begin note loop
  while (*_tunePntr)
  {
    // first, get note duration, if available
    _num = 0;
    while(isdigit(*_tunePntr))
    {
      _num = (_num * 10) + (*_tunePntr++ - '0');
    }
    
    if(_num) _duration = _wholenote / _num;
    else _duration = _wholenote / _default_dur;  // we will need to check if we are a dotted note after

    // now get the note
    _note = 0;

    switch(*_tunePntr)
    {
      case 'c':
        _note = 1;
        break;
      case 'd':
        _note = 3;
        break;
      case 'e':
        _note = 5;
        break;
      case 'f':
        _note = 6;
        break;
      case 'g':
        _note = 8;
        break;
      case 'a':
        _note = 10;
        break;
      case 'b':
        _note = 12;
        break;
      case 'p':
      default:
        _note = 0;
    }
    _tunePntr++;

    // now, get optional '#' sharp
    if(*_tunePntr == '#')
    {
      _note++;
      _tunePntr++;
    }

    // now, get optional '.' dotted note
    if(*_tunePntr == '.')
    {
      _duration += _duration/2;
      _tunePntr++;
    }
  
    // now, get scale
    if(isdigit(*_tunePntr))
    {
      _scale = *_tunePntr - '0';
      _tunePntr++;
    }
    else
    {
      _scale = _default_oct;
    }

    _scale += OCTAVE_OFFSET;

    if(*_tunePntr == ',')
      _tunePntr++;       // skip comma for next note (or we may be at the end)

    // now play the next note
    if(_note)
    {
        tone(notes[(_scale - 4) * 12 + _note], _duration);

        //Serial.print(F("Note: "));
        //Serial.print(_note, DEC);
        //Serial.print(F(", duration: "));
        //Serial.println(_duration, DEC);


      //delay(duration);
	  _delay_ms(_duration);
        noTone();
    }
    else
    {
      //very low frequency just for 'playing' pauses as we need to keep timer running
      //tone(10000, _duration);
          noTone();  
	  _delay_ms(_duration);
    }
//    return true;
  }
  //else return false;
return false;
}

