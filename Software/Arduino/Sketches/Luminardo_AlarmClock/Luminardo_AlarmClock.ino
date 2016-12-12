/*
 Luminardo Alarm Clock
 
 Copyright (c) 2014 Dmitry Pakhomenko.
 dmitryp@magictale.com
 http://magictale.com
 
 This example code is in the public domain.
 */

#define LUMINARDO_REV 2 

#include <Board.h>                       //Generic Luminardo definitions
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <DS1338.h>                      //RTC
#include <OneWire.h>                     //OneWire temperature sensor
#include <MVFDPanel_16S8D.h>             //VFD display
#include "AlarmClock.h"                  //AlarmClock specific definitions
#include "Messages.h"                    //Collection of string constants
#include "Utils.h"                       //Conversion utils and helpers
#include "ToneHelper.h"

void (*boot_loader_start)(void) = (void(*)(void))0x1F800; 

MVFD_16S8D vfd(VFD_CS_PIN, 
VFD_SCLK_PIN, VFD_DATAI_PIN, 
VFD_SHDN_PIN, VFD_PWR_EN_PIN);           //VFD display

OneWire ds(TEMP_SNSR_PIN);               //DS18B20 - a 1Wire temperature sensor from Dallas Semiconductor
volatile uint8_t pirEvent;  

uint8_t redValue;                        //current red value for front LED
uint8_t greenValue;                      //current green value for front LED
uint8_t blueValue;                       //current blue value for front LED
bool redValueUp;
bool greenValueUp;
bool blueValueUp;

uint8_t sysState;                        //current system state (mode)
uint8_t stateCntr;                       //counter for displaying current state (in sec) - when becomes
uint16_t mainLoopCntr;
//zero display state changes to the next one
uint8_t dsState;                         //temperature sensor state (to avoid explicit usage of delays)
RtcTimeType rtc;                         //current time
AlarmType alarm;                         //current alarm
float temperature;                       //current temperature
uint8_t actModeTimer;                    //seconds remaining before going to standby mode
EepromParamsType eepromParams EEMEM;     //RC keys allocations
uint8_t dsAddr[8];  //temperature sensor address
uint8_t dsData[12]; //temperature sensor data buffer
uint8_t alarmedSec;

void switchVFD(uint8_t state)
{
  cli();
  sysState = state;
  stateCntr = STD_DISP_TIME;
  vfd.standby(false);
  vfd.powerdown(false);
  vfd.reset();
  actModeTimer = SECONDS_IN_ACTIVE_MODE;
  sei();
}

// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(57600);  

//  shortBeep();  

  //Initialize RTC
  rtc.year = -1;
  if (RTClock.init() == 0)
  {
    RTClock.setCtrlReg(1 << DS1338_CTRL_REG_SQWE);
    RTClock.setClockHalt();
    RTClock.resetClockHalt();
  }

  //Initialize temperature sensor variables
  temperature = FAILED_TEMPERATURE;
  dsState = dsIdle;

  //Initialize VFD panel
  vfd.initLED(VFD_R_PIN, VFD_G_PIN, VFD_B_PIN);
  vfd.initLightSensor(VFD_LIGHT_SNR_PIN);
  //vfd.initTone(VFD_SPRK_PIN);
  vfd.initIR_RC(VFD_IR_REC_PIN, VFD_SPRK_PIN);

  redValue = 0;
  greenValue = 127;
  blueValue = 0;
  
  vfd.setLED(redValue, greenValue, blueValue);

  pinMode(VFD_DATAO_PIN, INPUT);
  switchVFD(sysGreetings);

  serial_print_p(PRODUCT_NAME);
  serial_print_p(SPACE_CHAR);
  serial_print_p(FIRMWARE_REV);
  serial_print_p(SPACE_CHAR);
  serial_println_p(FIRMWARE_DATE);

  mainLoopCntr = 0;
  
  //Initialize PIR sensor  
  pinMode(PIR_PIN, INPUT);
  EICRA |= (1 << ISC21) | (1 << ISC20);
  EIMSK |= (1 << INT2);
  pirEvent = false;

  RestoreAlarm();

  vfd.spkrOn(10000);
}

void toStandBy()
{
  cli();
  sysState = sysStandby;
  vfd.setLED(0, 0, 0);
  vfd.standby(true);
  sei();
}

void pulseLED(uint8_t &ledVal, bool &ledValueUp)
{
  if (ledVal != 0)
  {
      if (ledValueUp)
      {
          if (ledVal >= 250)
          {
              ledVal -= 5;
              ledValueUp = false;
          } else ledVal += 5;
      }else
      {
          if (ledVal <= 6)
          {
              ledVal += 5;
              ledValueUp = true;
          } else ledVal -= 5;
      }
  }
}

void pulseLEDs()
{
   if (sysState == sysTimeDisp || sysState == sysDayOfWeekDisp || sysState == sysExtTempDisp || sysState == sysIntTempDisp || sysState == sysDateDisp)
   { 
      blueValue++;
      if (blueValue > 60)
      {
          blueValue = 0;  
      }
      if (blueValue < 30)
      {
         vfd.setLED(blueValue/1.17, blueValue/8.6, blueValue);
      }else
      {
         uint8_t newVal = (30 - (blueValue - 30));
         vfd.setLED(newVal/1.17, newVal/8.6, newVal);
      }
   }else if (sysState == sysSetHours || sysState == sysSetMinutes || sysState == sysResetSeconds || sysState == sysSetYear || sysState == sysSetMonth ||
       sysState == sysSetDay || sysState == sysSetDayOfWeek || sysState == sysSetAlarmHours || sysState == sysSetAlarmMinutes || sysState == sysSetAlarmState)
   {
       vfd.setLED(127, 127, 0);
   }else if (sysState == sysWaitForSetup || sysState == sysRCSetupLeft || sysState == sysRCSetupRight || sysState == sysRCSetupUp || sysState == sysRCSetupDown ||
       sysState == sysRCSetupSettings)
   {
       vfd.setLED(0, 255, 0);
   }else if (sysState == sysStandby)
   {
       ;
   }
}

uint8_t isCodeEqual(decode_results *results, uint32_t* addr)
{
  eeprom_busy_wait();
  uint16_t rcType = eeprom_read_word(&eepromParams.rcType);
  if (rcType != results->decode_type)
    return false;

  eeprom_busy_wait();
  uint32_t rcCmd = eeprom_read_dword(addr);
  return (rcCmd == results->value);
}

void printStoredRCComs()
{
  eeprom_busy_wait();
  uint16_t rcType = eeprom_read_word(&eepromParams.rcType);
  Serial.print(F("RCType")); 
  Serial.println(rcType, HEX); 

  eeprom_busy_wait();
  uint32_t rcCmd = eeprom_read_dword(&eepromParams.rcAction_Left_Code);
  Serial.print(F("rcAction_Left_Code")); 
  Serial.println(rcCmd, HEX); 

  eeprom_busy_wait();
  rcCmd = eeprom_read_dword(&eepromParams.rcAction_Right_Code);
  Serial.print(F("rcAction_Right_Code")); 
  Serial.println(rcCmd, HEX); 

  eeprom_busy_wait();
  rcCmd = eeprom_read_dword(&eepromParams.rcAction_Up_Code);
  Serial.print(F("rcAction_Up_Code")); 
  Serial.println(rcCmd, HEX); 

  eeprom_busy_wait();
  rcCmd = eeprom_read_dword(&eepromParams.rcAction_Down_Code);
  Serial.print(F("rcAction_Down_Code")); 
  Serial.println(rcCmd, HEX); 

  eeprom_busy_wait();
  rcCmd = eeprom_read_dword(&eepromParams.rcAction_Settings_Code);
  Serial.print(F("rcAction_Settings_Code")); 
  Serial.println(rcCmd, HEX); 
  
}

void updateRCCmdInFlash(decode_results *results, uint32_t* addr)
{
  eeprom_busy_wait();
  uint16_t rcType = eeprom_read_word(&eepromParams.rcType);
  if (rcType != results->decode_type)
  {
    eeprom_busy_wait();
    eeprom_write_word(&eepromParams.rcType, results->decode_type);
  }

  eeprom_busy_wait();
  uint32_t rcCmd = eeprom_read_dword(addr);
  if (rcCmd != results->value)
  {
    eeprom_busy_wait();
    eeprom_write_dword(addr, results->value);
  }

  printStoredRCComs();

  delay(RCCMD_UPDATE_DELAY);
}



void defaultClockSettings()
{
  if (RTClock.init() == 0)
  {
    RTClock.getTime((RtcTimeType*)&rtc);
    rtc.seconds = MIN_SECONDS;
    rtc.minutes = MIN_MINUTES;
    rtc.hours = MIN_HOURS;
    rtc.day = MIN_DAY;
    rtc.weekday = MIN_WEEKDAY;
    rtc.month = MIN_MONTH;
    rtc.year = DEF_YEAR;
    RTClock.setTime((RtcTimeType*)&rtc);
  }
}


void dispAlarm()
{
  vfd.clearFrame();
  vfd.print_f_p(ALRM);
  bcdToVFD(binToBcd(alarm.hours), 4 * VFD_BYTES_PER_DIGIT, false, true);
  bcdToVFD(binToBcd(alarm.minutes), 6 * VFD_BYTES_PER_DIGIT, true, false);
  vfd.flipFrame();
}

void dispAlarmState()
{
  vfd.clearFrame();

  if (alarm.is_active) 
    vfd.print_f_p(ALRM_ON);
  else 
    vfd.print_f_p(ALRM_OFF);

  vfd.flipFrame();
}

void dispTime()
{
  vfd.clearFrame();
  bcdToVFD(rtc.hours, 0, false, false);
  bcdToVFD(rtc.minutes, 2 * VFD_BYTES_PER_DIGIT, true, false);
  bcdToVFD(rtc.seconds, 4 * VFD_BYTES_PER_DIGIT, true, false);
  vfd.flipFrame();
}

void dispDate()
{
  vfd.clearFrame();
  uint8_t idx = 0;
  idx = bcdToVFD(rtc.day, idx, false, true);
  idx += VFD_BYTES_PER_DIGIT;
  vfd.write_f(0x20, idx, false);

  uint8_t mon = ((rtc.month >> 4 && 0xF) * 10) + (rtc.month & 0xF);

  idx += VFD_BYTES_PER_DIGIT;
  vfd.write_f(pgm_read_byte(&MONTHS[mon-1][0]), idx, false);
  idx += VFD_BYTES_PER_DIGIT;
  vfd.write_f(pgm_read_byte(&MONTHS[mon-1][1]), idx, false);
  idx += VFD_BYTES_PER_DIGIT;
  vfd.write_f(pgm_read_byte(&MONTHS[mon-1][2]), idx, false);
  idx += VFD_BYTES_PER_DIGIT;
  vfd.write_f(0x20, idx, false);
  vfd.flipFrame();
}

void dispDayOfWeek()
{
  vfd.clearFrame();
  vfd.print_f_p((PGM_P)pgm_read_word(&DAYSOFWEEK[rtc.weekday - 1]));
  vfd.flipFrame();
}

void noDataToVFD()
{
  vfd.setCur(0);
  for (uint8_t i = 0; i < VFD_DIGITS-1; i++) 
    vfd.write(DASH);
  vfd.write(SPACE_CHR);
}

void NoTempToVFD()
{
  vfd.setCur(0);
  vfd.write(DASH);
  vfd.write(DASH);
  vfd.write(DEGREE);
  vfd.write('C');
  vfd.write(SPACE_CHR);
  vfd.write(SPACE_CHR);
  vfd.write(SPACE_CHR);
}

void SaveAlarm()
{
  RTClock.writeBlock((uint8_t*)&alarm, DS1338_USER_AREA, sizeof(alarm));
}

void RestoreAlarm()
{
  RTClock.readBlock((uint8_t*)&alarm, DS1338_USER_AREA, sizeof(alarm));
}

void check4Alarm()
{
  if (alarm.is_active && binToBcd(alarm.hours) == rtc.hours && binToBcd(alarm.minutes) == rtc.minutes && sysState != sysAlarmDisp)
  {
    /*sysState = sysAlarmDisp;
    stateCntr = STD_DISP_TIME * 2;
    vfd.print_f_p(WAKEUP);
    vfd.setFlashAttr(VFD_DIGITS, VFD_FLASH_ALL);
    vfd.flipFrame();*/

    sysState = sysAlarmDisp;
    stateCntr = STD_DISP_TIME * 2;

    alarmedSec = rtc.seconds;
    vfd.spkrOn(10000);
    
    vfd.clearFrame();
    bcdToVFD(rtc.hours, 2 * VFD_BYTES_PER_DIGIT, false, false);
    bcdToVFD(rtc.minutes, 4 * VFD_BYTES_PER_DIGIT, true, false);
    vfd.setFlashAttr(VFD_DIGITS, VFD_FLASH_ALL);
    vfd.flipFrame();    

    //Serial.println(F("Alarm!"));     
  }
}

void IRDecode(decode_results *results) 
{
  Serial.println((long)results->value, HEX);

  if (sysState == sysWaitForSetup)
  {
    sysState = sysRCSetupLeft;
    vfd.print_f_p(RC_LEFT);
    stateCntr = STD_DISP_TIME * 5;
    delay(RCCMD_UPDATE_DELAY);
  }
  else if (sysState == sysRCSetupLeft)
  {
    sysState = sysRCSetupRight;
    vfd.print_f_p(RC_RIGHT);
    stateCntr = STD_DISP_TIME * 5;
    updateRCCmdInFlash(results, &eepromParams.rcAction_Left_Code);
  }
  else if (sysState == sysRCSetupRight)
  {
    sysState = sysRCSetupUp;
    vfd.print_f_p(RC_UP);
    stateCntr = STD_DISP_TIME * 5;
    updateRCCmdInFlash(results, &eepromParams.rcAction_Right_Code);
  }
  else if (sysState == sysRCSetupUp)
  {
    sysState = sysRCSetupDown;
    vfd.print_f_p(RC_DOWN);
    stateCntr = STD_DISP_TIME * 5;
    updateRCCmdInFlash(results, &eepromParams.rcAction_Up_Code);
  }
  else if (sysState == sysRCSetupDown)
  {
    sysState = sysRCSetupSettings;
    vfd.print_f_p(RC_SETTINGS);
    stateCntr = STD_DISP_TIME * 5;
    updateRCCmdInFlash(results, &eepromParams.rcAction_Down_Code);
  }
  else if (sysState == sysRCSetupSettings)
  {
    sysState = sysTimeDisp;
    stateCntr = STD_DISP_TIME;
    vfd.setFlashAttr(VFD_DIGITS, VFD_FLASH_NONE);

    updateRCCmdInFlash(results, &eepromParams.rcAction_Settings_Code);
  }
  else if (sysState == sysTimeDisp || 
    sysState == sysDayOfWeekDisp ||
    sysState == sysExtTempDisp ||
    sysState == sysDateDisp)
  {
    if (isCodeEqual(results, &eepromParams.rcAction_Settings_Code))
    {
      sysState = sysSetAlarmHours;
      stateCntr = STD_DISP_TIME * 5;
      dispAlarm();
      vfd.setFlashAttr(RIGHT_DIGIT_IDX-4, 1);
      vfd.setFlashAttr(RIGHT_DIGIT_IDX-5, 1);      
      
      /*sysState = sysSetHours;
      stateCntr = STD_DISP_TIME * 5;
      if (rtc.year == (uint16_t)-1) defaultClockSettings();
      dispTime();
      vfd.setFlashAttr(RIGHT_DIGIT_IDX, 1);
      vfd.setFlashAttr(RIGHT_DIGIT_IDX-1, 1);*/
    }
  }
  else if (sysState == sysSetAlarmHours)
  {
    stateCntr = STD_DISP_TIME * 5;
    if (isCodeEqual(results, &eepromParams.rcAction_Up_Code))
    {
      uint8_t hours = alarm.hours + 1;
      if (hours > MAX_HOURS) hours = MIN_HOURS;
      alarm.hours = hours;

      SaveAlarm();

  //Serial.print(F("rtc.hour: "));       
  //Serial.println(hours, HEX); 
      
  //Serial.print(F("binToBcd(rtc.hour): "));       
  //Serial.println(binToBcd(hours), HEX); 

      dispAlarm();
      delay(RCCMD_DEBOUNCER);
    }
    else if (isCodeEqual(results, &eepromParams.rcAction_Down_Code))
    {
      uint8_t hours = alarm.hours;
      if (hours > MIN_HOURS) hours--; 
      else hours = MAX_HOURS;
      alarm.hours = hours;

      SaveAlarm();
      
      dispAlarm();
      delay(RCCMD_DEBOUNCER);
    }
    else if (isCodeEqual(results, &eepromParams.rcAction_Right_Code))
    {
      sysState = sysSetAlarmMinutes;
      vfd.setFlashAttr(VFD_DIGITS, VFD_FLASH_NONE);
      vfd.setFlashAttr(RIGHT_DIGIT_IDX-6, 1);
      vfd.setFlashAttr(RIGHT_DIGIT_IDX-7, 1);
      delay(RCCMD_DEBOUNCER);
    }
  }
  else if (sysState == sysSetAlarmMinutes)
  {
    stateCntr = STD_DISP_TIME * 5;
    if (isCodeEqual(results, &eepromParams.rcAction_Up_Code))
    {
      uint8_t minutes = alarm.minutes + 1;
      if (minutes > MAX_MINUTES) minutes = MIN_MINUTES;
      alarm.minutes = minutes;

      SaveAlarm();
      
      dispAlarm();
      delay(RCCMD_DEBOUNCER);
    }
    else if (isCodeEqual(results, &eepromParams.rcAction_Down_Code))
    {
      uint8_t minutes = alarm.minutes;
      if (minutes > MIN_MINUTES) minutes--; 
      else minutes = MAX_MINUTES;
      alarm.minutes = minutes;

      SaveAlarm();
      
      dispAlarm();
      delay(RCCMD_DEBOUNCER);
    }
    else if (isCodeEqual(results, &eepromParams.rcAction_Left_Code))
    {
      sysState = sysSetAlarmHours;
      vfd.setFlashAttr(VFD_DIGITS, VFD_FLASH_NONE);
      vfd.setFlashAttr(RIGHT_DIGIT_IDX-4, 1);
      vfd.setFlashAttr(RIGHT_DIGIT_IDX-5, 1);
      delay(RCCMD_DEBOUNCER);
    }
    else if (isCodeEqual(results, &eepromParams.rcAction_Right_Code))
    {
      /*sysState = sysSetHours;
      stateCntr = STD_DISP_TIME * 5;
      if (rtc.year == (uint16_t)-1) defaultClockSettings();
      dispTime();
      vfd.setFlashAttr(RIGHT_DIGIT_IDX, 1);
      vfd.setFlashAttr(RIGHT_DIGIT_IDX-1, 1);*/

      sysState = sysSetAlarmState;
      stateCntr = STD_DISP_TIME * 5;
      dispAlarmState();
      vfd.setFlashAttr(RIGHT_DIGIT_IDX-5, 1);
      vfd.setFlashAttr(RIGHT_DIGIT_IDX-6, 1);
      vfd.setFlashAttr(RIGHT_DIGIT_IDX-7, 1);      
    }
   
  }
  else if (sysState == sysSetAlarmState)
  {
    stateCntr = STD_DISP_TIME * 5;
    if (isCodeEqual(results, &eepromParams.rcAction_Up_Code) || isCodeEqual(results, &eepromParams.rcAction_Down_Code))
    //if (isCodeEqual(results, &eepromParams.rcAction_Up_Code))
    {
      if (alarm.is_active) alarm.is_active = false;
      else alarm.is_active = true;
      

      SaveAlarm();

      Serial.print(F("alarm.is_active: ")); 
      Serial.println(alarm.is_active, HEX);       
      
      dispAlarmState();
      delay(RCCMD_DEBOUNCER);
    }
    else if (isCodeEqual(results, &eepromParams.rcAction_Right_Code))
    {
      sysState = sysSetHours;
      stateCntr = STD_DISP_TIME * 5;
      if (rtc.year == (uint16_t)-1) defaultClockSettings();
      dispTime();
      vfd.setFlashAttr(VFD_DIGITS, VFD_FLASH_NONE);
      vfd.setFlashAttr(RIGHT_DIGIT_IDX, 1);
      vfd.setFlashAttr(RIGHT_DIGIT_IDX-1, 1);      
    }
    else if (isCodeEqual(results, &eepromParams.rcAction_Left_Code))
    {
      sysState = sysSetAlarmHours;
      stateCntr = STD_DISP_TIME * 5;
      dispAlarm();
      vfd.setFlashAttr(VFD_DIGITS, VFD_FLASH_NONE);      
      vfd.setFlashAttr(RIGHT_DIGIT_IDX-4, 1);
      vfd.setFlashAttr(RIGHT_DIGIT_IDX-5, 1);      
    }
  }
  
  else if (sysState == sysSetHours)
  {
    stateCntr = STD_DISP_TIME * 5;
    if (isCodeEqual(results, &eepromParams.rcAction_Up_Code))
    {
      uint8_t hours = bcdToBin(rtc.hours) + 1;
      if (hours > MAX_HOURS) hours = MIN_HOURS;
      rtc.hours = binToBcd(hours);

      if (RTClock.init() == 0)
        RTClock.setTime((RtcTimeType*)&rtc);

      dispTime();
      delay(RCCMD_DEBOUNCER);
    }
    else if (isCodeEqual(results, &eepromParams.rcAction_Down_Code))
    {
      uint8_t hours = bcdToBin(rtc.hours);
      if (hours > MIN_HOURS) hours--; 
      else hours = MAX_HOURS;
      rtc.hours = binToBcd(hours);

      if (RTClock.init() == 0)
        RTClock.setTime((RtcTimeType*)&rtc);

      dispTime();
      delay(RCCMD_DEBOUNCER);
    }
    else if (isCodeEqual(results, &eepromParams.rcAction_Right_Code))
    {
      sysState = sysSetMinutes;
      vfd.setFlashAttr(VFD_DIGITS, VFD_FLASH_NONE);
      vfd.setFlashAttr(RIGHT_DIGIT_IDX-2, 1);
      vfd.setFlashAttr(RIGHT_DIGIT_IDX-3, 1);
      delay(RCCMD_DEBOUNCER);
    }
    else if (isCodeEqual(results, &eepromParams.rcAction_Left_Code))
    {
      /*
      sysState = sysSetAlarmHours;
      stateCntr = STD_DISP_TIME * 5;
      dispAlarm();
      vfd.setFlashAttr(VFD_DIGITS, VFD_FLASH_NONE);      
      vfd.setFlashAttr(RIGHT_DIGIT_IDX-4, 1);
      vfd.setFlashAttr(RIGHT_DIGIT_IDX-5, 1);      
      */
      sysState = sysSetAlarmState;
      stateCntr = STD_DISP_TIME * 5;
      dispAlarmState();
      vfd.setFlashAttr(VFD_DIGITS, VFD_FLASH_NONE);      
      vfd.setFlashAttr(RIGHT_DIGIT_IDX-5, 1);
      vfd.setFlashAttr(RIGHT_DIGIT_IDX-6, 1);
      vfd.setFlashAttr(RIGHT_DIGIT_IDX-7, 1);      
    }
  }
  else if (sysState == sysSetMinutes)
  {
    stateCntr = STD_DISP_TIME * 5;
    if (isCodeEqual(results, &eepromParams.rcAction_Up_Code))
    {
      uint8_t minutes = bcdToBin(rtc.minutes) + 1;
      if (minutes > MAX_MINUTES) minutes = MIN_MINUTES;
      rtc.minutes = binToBcd(minutes);

      if (RTClock.init() == 0)
        RTClock.setTime((RtcTimeType*)&rtc);

      dispTime();
      delay(RCCMD_DEBOUNCER);
    }
    else if (isCodeEqual(results, &eepromParams.rcAction_Down_Code))
    {
      uint8_t minutes = bcdToBin(rtc.minutes);
      if (minutes > MIN_MINUTES) minutes--; 
      else minutes = MAX_MINUTES;
      rtc.minutes = binToBcd(minutes);

      if (RTClock.init() == 0)
        RTClock.setTime((RtcTimeType*)&rtc);

      dispTime();
      delay(RCCMD_DEBOUNCER);
    }
    else if (isCodeEqual(results, &eepromParams.rcAction_Left_Code))
    {
      sysState = sysSetHours;
      vfd.setFlashAttr(VFD_DIGITS, VFD_FLASH_NONE);
      vfd.setFlashAttr(RIGHT_DIGIT_IDX, 1);
      vfd.setFlashAttr(RIGHT_DIGIT_IDX-1, 1);
      delay(RCCMD_DEBOUNCER);
    }
    else if (isCodeEqual(results, &eepromParams.rcAction_Right_Code))
    {
      sysState = sysResetSeconds;
      vfd.setFlashAttr(VFD_DIGITS, VFD_FLASH_NONE);
      vfd.setFlashAttr(RIGHT_DIGIT_IDX-4, 1);
      vfd.setFlashAttr(RIGHT_DIGIT_IDX-5, 1);
      delay(RCCMD_DEBOUNCER);
    }
  }
  else if (sysState == sysResetSeconds)
  {
    stateCntr = STD_DISP_TIME * 5;
    if (isCodeEqual(results, &eepromParams.rcAction_Up_Code) ||
      isCodeEqual(results, &eepromParams.rcAction_Down_Code))
    {
      rtc.seconds = MIN_SECONDS;
      if (RTClock.init() == 0)
        RTClock.setTime((RtcTimeType*)&rtc);

      dispTime();
      delay(RCCMD_DEBOUNCER);
    }
    else if (isCodeEqual(results, &eepromParams.rcAction_Left_Code))
    {
      sysState = sysSetMinutes;
      vfd.setFlashAttr(VFD_DIGITS, VFD_FLASH_NONE);
      vfd.setFlashAttr(RIGHT_DIGIT_IDX-2, 1);
      vfd.setFlashAttr(RIGHT_DIGIT_IDX-3, 1);
      delay(RCCMD_DEBOUNCER);
    }
    else if (isCodeEqual(results, &eepromParams.rcAction_Right_Code))
    {
      sysState = sysSetDayOfWeek;
      vfd.setFlashAttr(VFD_DIGITS, VFD_FLASH_ALL);
      dispDayOfWeek();
    }
  }
  else if (sysState == sysSetDayOfWeek)
  {
    stateCntr = STD_DISP_TIME * 5;
    if (isCodeEqual(results, &eepromParams.rcAction_Up_Code))
    {
      if (rtc.weekday == MAX_WEEKDAY) rtc.weekday = MIN_WEEKDAY;
      else rtc.weekday++;

      if (RTClock.init() == 0)
        RTClock.setTime((RtcTimeType*)&rtc);

      dispDayOfWeek();
      delay(RCCMD_DEBOUNCER);
    }
    if (isCodeEqual(results, &eepromParams.rcAction_Down_Code))
    {
      if (rtc.weekday == MIN_WEEKDAY) rtc.weekday = MAX_WEEKDAY;
      else rtc.weekday--;

      if (RTClock.init() == 0)
        RTClock.setTime((RtcTimeType*)&rtc);

      dispDayOfWeek();
      delay(RCCMD_DEBOUNCER);
    }
    else if (isCodeEqual(results, &eepromParams.rcAction_Left_Code))
    {
      sysState = sysResetSeconds;
      vfd.setFlashAttr(VFD_DIGITS, VFD_FLASH_NONE);
      vfd.setFlashAttr(RIGHT_DIGIT_IDX-4, 1);
      vfd.setFlashAttr(RIGHT_DIGIT_IDX-5, 1);
      delay(RCCMD_DEBOUNCER);
    }
    else if (isCodeEqual(results, &eepromParams.rcAction_Right_Code))
    {
      sysState = sysSetDay;
      vfd.setFlashAttr(VFD_DIGITS, VFD_FLASH_NONE);
      vfd.setFlashAttr(RIGHT_DIGIT_IDX, 1);
      vfd.setFlashAttr(RIGHT_DIGIT_IDX-1, 1);
      dispDate();
    }
  }
  else if (sysState == sysSetDay)
  {
    stateCntr = STD_DISP_TIME * 5;
    if (isCodeEqual(results, &eepromParams.rcAction_Up_Code))
    {
      uint8_t day = bcdToBin(rtc.day) + 1;

      if (day > 31) day = MIN_DAY;
      rtc.day = binToBcd(day);

      if (RTClock.init() == 0)
        RTClock.setTime((RtcTimeType*)&rtc);

      dispDate();
      delay(RCCMD_DEBOUNCER);
    }
    if (isCodeEqual(results, &eepromParams.rcAction_Down_Code))
    {
      uint8_t day = bcdToBin(rtc.day);
      if (day > MIN_DAY) day--; 
      else day = 31;
      rtc.day = binToBcd(day);

      if (RTClock.init() == 0)
        RTClock.setTime((RtcTimeType*)&rtc);

      dispDate();
      delay(RCCMD_DEBOUNCER);
    }
    else if (isCodeEqual(results, &eepromParams.rcAction_Left_Code))
    {
      sysState = sysSetDayOfWeek;
      vfd.setFlashAttr(VFD_DIGITS, VFD_FLASH_ALL);
      dispDayOfWeek();
      delay(RCCMD_DEBOUNCER);
    }
    else if (isCodeEqual(results, &eepromParams.rcAction_Right_Code))
    {
      sysState = sysSetMonth;
      vfd.setFlashAttr(VFD_DIGITS, VFD_FLASH_NONE);
      vfd.setFlashAttr(RIGHT_DIGIT_IDX-3, 1);
      vfd.setFlashAttr(RIGHT_DIGIT_IDX-4, 1);
      vfd.setFlashAttr(RIGHT_DIGIT_IDX-5, 1);
      dispDate();
      delay(RCCMD_DEBOUNCER);
    }
  }
  else if (sysState == sysSetMonth)
  {
    stateCntr = STD_DISP_TIME * 5;
    if (isCodeEqual(results, &eepromParams.rcAction_Up_Code))
    {
      uint8_t month = bcdToBin(rtc.month) + 1;
      if (month > MAX_MONTH) month = MIN_MONTH;
      rtc.month = binToBcd(month);

      if (RTClock.init() == 0)
        RTClock.setTime((RtcTimeType*)&rtc);

      dispDate();
      delay(RCCMD_DEBOUNCER);
    }
    if (isCodeEqual(results, &eepromParams.rcAction_Down_Code))
    {
      uint8_t month = bcdToBin(rtc.month);
      if (month > MIN_MONTH) month--; 
      else month = MAX_MONTH;
      rtc.month = binToBcd(month);

      if (RTClock.init() == 0)
        RTClock.setTime((RtcTimeType*)&rtc);

      dispDate();
      delay(RCCMD_DEBOUNCER);
    }
    else if (isCodeEqual(results, &eepromParams.rcAction_Left_Code))
    {
      sysState = sysSetDay;
      vfd.setFlashAttr(VFD_DIGITS, VFD_FLASH_NONE);
      vfd.setFlashAttr(RIGHT_DIGIT_IDX, 1);
      vfd.setFlashAttr(RIGHT_DIGIT_IDX-1, 1);
      dispDate();
      delay(RCCMD_DEBOUNCER);
    }
    else if (isCodeEqual(results, &eepromParams.rcAction_Right_Code))
    {
    }
  }
  vfd.resumeIR_RC();
}

float getTemperature(uint8_t* data)
{
  int HighByte, LowByte, TReading, SignBit, Tc_100;
  float res;

  LowByte = data[0];
  HighByte = data[1];
  TReading = (HighByte << 8) + LowByte;
  SignBit = TReading & 0x8000;  // test most sig bit
  if (SignBit) // negative
    TReading = (TReading ^ 0xffff) + 1; // 2's comp

  Tc_100 = (6 * TReading) + TReading / 4;    // multiply by (100 * 0.0625) or 6.25

  res = Tc_100 / 100;
  if (SignBit) res *= -1;

  return res;
}

//PIR interrupt
SIGNAL(INT2_vect)
{
  cli();  
  pirEvent = true;
  sei();
}

// the loop routine runs over and over again forever:
void loop() {

  while (1)
  {
    {
      switch (sysState)
      {
      case sysSelfTest:
        {
          if (vfd.testStep() == vfd.COMPLETED)
          {
            boot_loader_start();
          }
        }
        break;
      case sysGreetings:
        if (stateCntr == 0)
        {
          sysState = sysWaitForSetup;
          stateCntr = STD_DISP_TIME * 2;
          vfd.print_f_p(RC_SETUP);
          vfd.setFlashAttr(VFD_DIGITS, VFD_FLASH_ALL);
        }
        else
        {
          vfd.setCur(0);
          if (stateCntr > (STD_DISP_TIME / 3 * 2))
            vfd_print_p(HELLO);
          else if (stateCntr > STD_DISP_TIME / 3)
            vfd_print_p(MAGIC);
          else if  (stateCntr > 0)
            vfd_print_p(CLOCK);
        }
        break;
      case sysWaitForSetup:
      case sysRCSetupLeft:
      case sysRCSetupRight:
      case sysRCSetupUp:
      case sysRCSetupDown:
      case sysRCSetupSettings:
      case sysSetHours:
      case sysSetMinutes:
      case sysResetSeconds:
      case sysSetYear:
      case sysSetDayOfWeek:
      case sysSetMonth:
      case sysSetDay:
      case sysSetAlarmHours:
      case sysSetAlarmMinutes:
      case sysSetAlarmState:

        if (stateCntr == 0)
        {
          sysState = sysTimeDisp;
          stateCntr = STD_DISP_TIME;
          vfd.setFlashAttr(VFD_DIGITS, VFD_FLASH_NONE);
        }
        else
        {

          if (sysState == sysSetHours ||
            sysState == sysSetMinutes ||
            sysState == sysResetSeconds)
          {
            dispTime();
          }

          if (mainLoopCntr % 2 == 0)
          {
              vfd.flipFlashState();
              vfd.flipFrame();
          }
        }
        break;

      case sysAlarmDisp:
        {
          if (rtc.hours != binToBcd(alarm.hours) || rtc.minutes != binToBcd(alarm.minutes))
          {
              sysState = sysTimeDisp;
              stateCntr = STD_DISP_TIME;
              vfd.setFlashAttr(VFD_DIGITS, VFD_FLASH_NONE);
              //TODO: disable alarm
          }else
          {
              if (alarmedSec != rtc.seconds)
              {
                vfd.spkrOn(10000);
                alarmedSec = rtc.seconds;
              }
          }

          if (mainLoopCntr % 2 == 0)
          {
             vfd.flipFlashState();
             vfd.flipFrame();
          }
        }
        break;

      case sysTimeDisp:
        {
          if (rtc.year == (uint16_t)-1) noDataToVFD();
          else dispTime();

          if (stateCntr == 0)
          {
            sysState = sysDateDisp;
            stateCntr = STD_DISP_TIME;
          }

          check4Alarm();
        }
        break;

      case sysDayOfWeekDisp:
        if (rtc.year == (uint16_t)-1) noDataToVFD();
        else 
        {
          vfd.setCur(0);
          vfd_print_p((PGM_P)pgm_read_word(&DAYSOFWEEK[rtc.weekday - 1]));
          if (stateCntr == 0)
          {
            sysState = sysExtTempDisp;
            stateCntr = STD_DISP_TIME;
          }
        }
        check4Alarm();
        break;

      case sysExtTempDisp:
        if (temperature == FAILED_TEMPERATURE)NoTempToVFD();
        else 
        {
          vfd.setCur(0);
          if (temperature > 0) vfd.write('+');
          vfd.print((long)temperature, DEC);
          vfd.write(DEGREE);
          vfd.write('C');

          for (uint8_t i = 0; i < VFD_DIGITS-4; i++) 
            vfd.write(SPACE_CHR);
        }
        if (stateCntr == 0)
        {
          sysState = sysTimeDisp;
          stateCntr = STD_DISP_TIME;
        }
        check4Alarm();        
        break;

      case sysIntTempDisp:
        check4Alarm();          
        break;

      case sysDateDisp:
        if (rtc.year == (uint16_t)-1) noDataToVFD();
        else 
        {
          dispDate();
        }
        if (stateCntr == 0)
        {
          sysState = sysDayOfWeekDisp;
          stateCntr = STD_DISP_TIME;
        }
        check4Alarm();                  
        break;

      case sysIntroEffect:

        break;

      case sysStandby:

        break;

      case sysFailure:
        stateCntr = STD_DISP_TIME * 2;
        vfd.print_f_p(FAILURE);
        break;

      default:
        ;       
      }
    }

    //Getting current time
    if (mainLoopCntr % (ONE_SEC_IN_MAIN_LOOPS) == 0)
    {
      if (RTClock.init() == 0)
        RTClock.getTime((RtcTimeType*)&rtc);
    }

    //Getting current temperature
    if (mainLoopCntr % (ONE_SEC_IN_MAIN_LOOPS * 5) == 0)
    {
      if (dsState == dsIdle)
      {
        if ( ds.search(dsAddr)) 
        {
          if (ds.crc8(dsAddr, 7) == dsAddr[7])
          {
            if (dsAddr[0] == 0x28) 
            {
              ds.reset();
              ds.select(dsAddr);
              ds.write(0x44);
              dsState = dsTempConversion;
            }
          }
        }
        else 
        {
          ds.reset_search();
          dsState = dsInitDelay;
        }
      }
      else if (dsState == dsTempConversion)
      {
        ds.reset();
        ds.select(dsAddr);
        ds.write(0xBE);
        for (uint8_t i = 0; i < 9; i++) 
          dsData[i] = ds.read();

        if (ds.crc8(dsData, 8) == dsData[8])
        {
          temperature = getTemperature((uint8_t*)&dsData);
        }
        dsState = dsIdle;
      }
      else if (dsState == dsInitDelay)
      {
        dsState = dsIdle;
      }

      if (temperature != FAILED_TEMPERATURE)
      {
        if (temperature > 0) Serial.write('+');
        else if (temperature < 0) Serial.write('-');
        Serial.print((long)temperature, DEC);
        Serial.write('C');
      }
      Serial.println();

      Serial.print("Photocell: ");
      Serial.println((long)vfd.getLightSensorVal(), DEC);
    }

    //Handling RC commands
    decode_results* ir_res = vfd.getIR_RC_Code();
    if (ir_res != NULL) 
    {
      cli();
      actModeTimer = SECONDS_IN_ACTIVE_MODE;
      sei();

      if (sysState == sysStandby)
      {
        switchVFD(sysTimeDisp);
      }
      //shortBeep();
      IRDecode(ir_res);
    }

    if (actModeTimer == 0 &&  sysState != sysStandby)
    {
      uint8_t brightness = VFD_DIMMING_MAX;
      while (brightness != 0)
      {            
        vfd.displayOnCmd(brightness--);
        delay(500);
      }
      toStandBy();
    }

    if (pirEvent)
    {
      cli();
      pirEvent = false;
      actModeTimer = SECONDS_IN_ACTIVE_MODE;
      sei();

      if (sysState == sysStandby)
      {
        switchVFD(sysTimeDisp);
      }
    }

    //if (sysState == sysGreetings)
    //  longBeepAsync();

    //sleep_mode();
    delay(MAIN_LOOP_DELAY);

    if (sysState != sysStandby)
    {
        pulseLEDs();
    }

    mainLoopCntr++;
    if (mainLoopCntr % ONE_SEC_IN_MAIN_LOOPS == 0)
    {
      //1 Hz
      if (stateCntr != 0) stateCntr--; 
      if (actModeTimer != 0) actModeTimer--;
    }
  }
}
