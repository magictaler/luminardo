/*
  Luminardo RTC Test 
 
  This example code is in the public domain.
 */
 
 
#include <Board.h>
#include <DS1338.h>

volatile RtcTimeType rtc;       //current time
volatile uint8_t buf[10];


#define MIN_HOURS 4
#define MIN_MINUTES 0
#define MIN_MONTH 1
#define MAX_WEEKDAY 7
#define MIN_WEEKDAY 1
#define MIN_DAY 1
#define DEF_YEAR 0x2014
#define MIN_SECONDS 5


void readBuffer()
{
     RTClock.readBlock((uint8_t*)&buf, 0, 8);
     Serial.print(F("\r\nReading buffer, 1-st attempt "));
     for (int i = 0; i < 8; i++)
     {
       //Serial.print(buf[i], HEX);
       printBCD(buf[i], false);
       Serial.print(" ");
     }
     Serial.println();

     RTClock.readBlock((uint8_t*)&buf, 3, 8);     
     Serial.print(F("Reading buffer, 2-nd attempt "));
     for (int i = 0; i < 8; i++)
     {
       //Serial.print(buf[i], HEX);
       printBCD(buf[i], false);       
       Serial.print(" ");
     }
     Serial.println();
}

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 57600 bits per second:
  Serial.begin(57600);
  Serial.println(F("Luminardo RTC Sensor Test"));
  
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
//    if (RTClock.setTime((RtcTimeType*)&rtc) == 0)
//      Serial.println(F("RTC is successfully initialised"));
//    else
//      Serial.println(F("Failed to set time"));
  }else
      Serial.println(F("Failed to communicate with RTC"));
  
  RTClock.setCtrlReg(1 << DS1338_CTRL_REG_SQWE);
  RTClock.setClockHalt();
  readBuffer();
  RTClock.resetClockHalt();
  readBuffer();
}


void printBCD(uint8_t value, uint8_t suppLeadZero)
{
    uint8_t chrset = ((value >> 4) & 0xF) | 0x30;
//    if ((chrset == 0x30) && suppLeadZero) chrset = 0x20;
    Serial.write(chrset);
    Serial.write((value &0xF) | 0x30);
}

void printTime()
{
  printBCD(rtc.hours, false);  
  //Serial.print((int)rtc.hours, HEX);
  Serial.print(F(":"));
  printBCD(rtc.minutes, false);  
  //Serial.print((int)rtc.minutes, HEX);  
  Serial.print(F(":"));
  printBCD(rtc.seconds, false);  
  //Serial.print((int)rtc.seconds, HEX);
}

void printDate()
{
  printBCD(rtc.day, false);  
  Serial.print(F("."));
  printBCD(rtc.month, false);  
  Serial.print(F("."));    
  printBCD((uint8_t)(rtc.year - 0x2000), HEX);  
}

// the loop routine runs over and over again forever:
void loop() {
  if (RTClock.init() == 0)
  {
     RTClock.getTime((RtcTimeType*)&rtc);
     printTime();
     Serial.print(F(" "));
     printDate();
     Serial.println();     
     
     readBuffer();    

     
 }//else
        //Serial.println(F("Failed to communicate with RTC"));
        
   

  delay(5000);
}



