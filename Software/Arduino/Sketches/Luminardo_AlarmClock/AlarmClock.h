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
#define MAIN_LOOP_DELAY 200
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
    'M','A','Y'  }
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
    'O','C','T'  }
  ,
  {
    'N','O','V'  }
  ,
  {
    'D','E','C'  }
};

const PROGMEM char MON[] = "MONDAY ";
const PROGMEM char TUE[] = "TUESDAY";
const PROGMEM char WED[] = "WED-DAY";
const PROGMEM char THU[] = "THU-DAY";
const PROGMEM char FRI[] = "FRIDAY ";
const PROGMEM char SAT[] = "SAT-DAY";
const PROGMEM char SUN[] = "SUNDAY ";

static PGM_P DAYSOFWEEK[] PROGMEM = {
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
  sysSetHours,
  sysSetMinutes,
  sysResetSeconds,
  sysSetYear,
  sysSetMonth,
  sysSetDay,
  sysSetDayOfWeek,
  sysSetAlarm,
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

#endif


