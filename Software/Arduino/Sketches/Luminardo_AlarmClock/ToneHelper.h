/*
 Luminardo Alarm Clock
 
 Copyright (c) 2014 Dmitry Pakhomenko.
 dmitryp@magictale.com
 http://magictale.com
 
 This example code is in the public domain.
 */

#ifndef ToneHelper_h
#define ToneHelper_h


volatile uint8_t beepState;

extern MVFD_16S8D vfd;

void longBeepAsync()
{
  if (beepState == 1 || beepState == 2 ||
    beepState == 5 || beepState == 6)
  {
    vfd.playTone(NOTE_FS7, 100);
    //analogWrite(VFD_SPRK_PIN, 50);  
  }

  if (beepState < 7) beepState++;
}

#endif


