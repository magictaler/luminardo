/* ToneEx.cpp

  A mixture of Tone Generator library by Brett Hagman, D Mellis, M Sproul 
  with RTTTL (RingTone Text Transfer Language) songs from
  http://code.google.com/p/rogue-code/wiki/ToneLibraryDocumentation

  Compiled into a class by Dmitry Pakhomenko
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

#ifndef ToneEx_H
#define ToneEx_H

#include "Settings.h"

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978


#if defined(__AVR_ATmega8__) || defined(__AVR_ATmega128__)
#define TCCR2A TCCR2
#define TCCR2B TCCR2
#define COM2A1 COM21
#define COM2A0 COM20
#define OCR2A OCR2
#define TIMSK2 TIMSK
#define OCIE2A OCIE2
#define TIMER2_COMPA_vect TIMER2_COMP_vect
#define TIMSK1 TIMSK
#endif

// MLS: This does not make sense, the 3 options are the same
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)

#define AVAILABLE_TONE_PINS 1

const uint8_t PROGMEM tone_pin_to_timer_PGM[] = { 3 /*, 3, 4, 5, 1, 0 */ };
static uint8_t tone_pins[AVAILABLE_TONE_PINS] = { 255 /*, 255, 255, 255, 255, 255 */ };

#elif defined(__AVR_ATmega8__)

#define AVAILABLE_TONE_PINS 1

const uint8_t PROGMEM tone_pin_to_timer_PGM[] = { 3 /*, 1 */ };
static uint8_t tone_pins[AVAILABLE_TONE_PINS] = { 255 /*, 255 */ };

#else

#define AVAILABLE_TONE_PINS 1
#define OCTAVE_OFFSET 0

// Leave timer 0 to last.
const uint8_t PROGMEM tone_pin_to_timer_PGM[] = { 3 /*, 1, 0 */ };
static uint8_t tone_pins[AVAILABLE_TONE_PINS] = { 255 /*, 255, 255 */ };

#endif

// timerx_toggle_count:
//  > 0 - duration specified
//  = 0 - stopped
//  < 0 - infinitely (until stop() method called, or new play() called)

#if !defined(__AVR_ATmega8__)
static volatile long timer0_toggle_count;
static volatile uint8_t *timer0_pin_port;
static volatile uint8_t timer0_pin_mask;
#endif

static volatile long timer1_toggle_count;
static volatile uint8_t *timer1_pin_port;
static volatile uint8_t timer1_pin_mask;
static volatile long timer2_toggle_count;
static volatile uint8_t *timer2_pin_port;
static volatile uint8_t timer2_pin_mask;

#if defined(TIMSK3)
static volatile long timer3_toggle_count;
static volatile uint8_t *timer3_pin_port;
static volatile uint8_t timer3_pin_mask;
#endif

#if defined(TIMSK4)
static volatile long timer4_toggle_count;
static volatile uint8_t *timer4_pin_port;
static volatile uint8_t timer4_pin_mask;
#endif

#if defined(TIMSK5)
static volatile long timer5_toggle_count;
static volatile uint8_t *timer5_pin_port;
static volatile uint8_t timer5_pin_mask;
#endif

static int notes[] = { 0,
  NOTE_C4, NOTE_CS4, NOTE_D4, NOTE_DS4, NOTE_E4, NOTE_F4, NOTE_FS4, NOTE_G4, NOTE_GS4, NOTE_A4, NOTE_AS4, NOTE_B4,
  NOTE_C5, NOTE_CS5, NOTE_D5, NOTE_DS5, NOTE_E5, NOTE_F5, NOTE_FS5, NOTE_G5, NOTE_GS5, NOTE_A5, NOTE_AS5, NOTE_B5,
  NOTE_C6, NOTE_CS6, NOTE_D6, NOTE_DS6, NOTE_E6, NOTE_F6, NOTE_FS6, NOTE_G6, NOTE_GS6, NOTE_A6, NOTE_AS6, NOTE_B6,
  NOTE_C7, NOTE_CS7, NOTE_D7, NOTE_DS7, NOTE_E7, NOTE_F7, NOTE_FS7, NOTE_G7, NOTE_GS7, NOTE_A7, NOTE_AS7, NOTE_B7
};


class ToneEx
{
    public:

        ToneEx();
        void init(uint8_t pin);
        int8_t toneBegin();
        void tone(unsigned int frequency, unsigned long duration);
        void noTone();
        bool isPlaying();
        void abortPlay();
        void playRtttl(char *p);
        bool pushNextNote();
        void disableTimer(uint8_t _timer);

protected: 

        byte _default_dur;
        byte _default_oct;
        int _bpm;
        int _num;
        long _wholenote;
        long _duration;
        byte _note;
        byte _scale;
        char* _tunePntr;
        bool _isPlaying;
        uint8_t _pin;

};

// Preinstantiate Objects //////////////////////////////////////////////////////
static ToneEx Tone;

#endif
