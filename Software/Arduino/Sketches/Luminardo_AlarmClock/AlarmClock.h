/*
 Luminardo Alarm Clock
 
 Copyright (c) 2014 Dmitry Pakhomenko.
 dmitryp@magictale.com
 http://magictale.com
 
 This example code is in the public domain.
 */

#ifndef AlarmClock_h
#define AlarmClock_h

#define STD_DISP_TIME 3
#define SECONDS_IN_ACTIVE_MODE 180
#define MAIN_LOOP_DELAY 100
#define ONE_SEC_IN_MAIN_LOOPS (1000 / MAIN_LOOP_DELAY)
#define RCCMD_UPDATE_DELAY 1000
#define RCCMD_DEBOUNCER 300
#define DASH 0x21
#define DEGREE 0x5E
#define SPACE_CHR 0x20
#define RIGHT_DIGIT_IDX (VFD_DIGITS - 1)

#define MAX_HOURS 23
#define MAX_MINUTES 59
#define MAX_MONTH 12
#define MIN_HOURS 0
#define MIN_MINUTES 0
#define MIN_MONTH 1
#define MAX_WEEKDAY 7
#define MIN_WEEKDAY 1
#define MIN_DAY 1
#define DEF_YEAR 0x2012
#define MIN_SECONDS 0

#define DS1338_USER_AREA 8

const PROGMEM uint8_t MONTHS[12][3] = {
  {
    'J','A','N'  }
  ,
  {
    'F','E','B'  }
  ,
  {
    'M','A','R'  }
  ,
  {
    'A','P','R'  }
  ,
  {
    'M','A','I'  }
  ,
  {
    'J','U','N'  }
  ,
  {
    'J','U','L'  }
  ,
  {
    'A','U','G'  }
  ,
  {
    'S','E','P'  }
  ,
  {
    'O','K','T'  }
  ,
  {
    'N','O','V'  }
  ,
  {
    'D','E','Z'  }
};

const char MON[] PROGMEM = " MONTAG ";
const char TUE[] PROGMEM = "DIENSTAG";
const char WED[] PROGMEM = "MITTWOCH";
const char THU[] PROGMEM = "DONNERST";
const char FRI[] PROGMEM = "FREITAG ";
const char SAT[] PROGMEM = "SONNABNT";
const char SUN[] PROGMEM = "SONNTAG ";

const char* const DAYSOFWEEK[] PROGMEM = {
  MON,
  TUE,
  WED,
  THU,
  FRI,
  SAT,
  SUN
};

enum enum_SysState
{
  sysSelfTest,
  sysGreetings,
  sysTimeDisp,
  sysDayOfWeekDisp,
  sysExtTempDisp,
  sysIntTempDisp,
  sysDateDisp,
  sysAlarmDisp,
  sysSetHours,
  sysSetMinutes,
  sysResetSeconds,
  sysSetYear,
  sysSetMonth,
  sysSetDay,
  sysSetDayOfWeek,
  sysSetAlarmHours,
  sysSetAlarmMinutes,
  sysSetAlarmState,  
  sysIntroEffect,
  sysStandby,
  sysWaitForSetup,
  sysRCSetupLeft,
  sysRCSetupRight,
  sysRCSetupUp,
  sysRCSetupDown,
  sysRCSetupSettings,
  sysFailure
};

typedef struct struct_EepromParams
{       
  uint16_t rcType;
  uint32_t rcAction_Left_Code;
  uint32_t rcAction_Right_Code;
  uint32_t rcAction_Up_Code;
  uint32_t rcAction_Down_Code;
  uint32_t rcAction_Settings_Code;
} 
EepromParamsType;

typedef struct struct_Alarm
{  
  uint8_t minutes;
  uint8_t hours;
  bool is_active;
} AlarmType;


#endif


