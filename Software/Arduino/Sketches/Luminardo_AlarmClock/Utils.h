/*
 Luminardo Alarm Clock
 
 Copyright (c) 2014 Dmitry Pakhomenko.
 dmitryp@magictale.com
 http://magictale.com
 
 This example code is in the public domain.
 */

#ifndef Utils_h
#define Utils_h

extern MVFD_16S8D vfd;

void serial_print_p(const prog_char str[])
{
  char c;
  if(!str) return;
  while((c = pgm_read_byte(str++)))
    Serial.write(c);
}

void serial_println_p(const prog_char str[])
{
  serial_print_p(str);
  Serial.println();
}

void vfd_print_p(const prog_char str[])
{
  char c;
  if(!str) return;
  while((c = pgm_read_byte(str++)))
    vfd.write(c);
}

uint8_t bcdToBin(uint8_t value)
{
  uint8_t hex = 10;
  hex *= (value & 0xF0) >> 4;
  hex += (value & 0x0F);
  return hex; 
}

uint8_t binToBcd(uint8_t value)
{
  uint8_t MSD = 0;
  while (value >= 10)
  {
    value -= 10;
    MSD += 0x10;
  }
  return MSD + value;
}

uint8_t bcdToVFD(uint8_t value, uint8_t dstIndex, uint8_t dispColumn, uint8_t suppLeadZero)
{
  uint8_t chrset = ((value >> 4) & 0xF) + 0x30;
  if ((chrset == 0x30) && suppLeadZero) chrset = 0x20;
  vfd.write_f(chrset, dstIndex, dispColumn);
  dstIndex += VFD_BYTES_PER_DIGIT;
  vfd.write_f((value &0xF) + 0x30, dstIndex, dispColumn);
  return dstIndex;
}


#endif


