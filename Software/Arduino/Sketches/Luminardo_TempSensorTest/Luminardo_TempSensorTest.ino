/*
  Luminardo Temperature Sensor Test 
 
  This example code is in the public domain.
 */
 
#include <Board.h>
#include <OneWire.h>

OneWire ds(TEMP_SNSR_PIN);      //DS18B20 - a 1Wire temperature sensor from Dallas Semiconductor
volatile uint8_t dsState;       //temperature sensor state (to avoid explicit usage of delays)
volatile float temperature;     //current temperature

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 57600 bits per second:
  Serial.begin(57600);
  Serial.println(F("Luminardo Temperature Sensor Test"));
  dsState = dsIdle;  
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


// the loop routine runs over and over again forever:
void loop() {
  uint8_t dsAddr[8];  //temperature sensor address
  uint8_t dsData[12]; //temperature sensor data buffer
  
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
    }else 
    {
      ds.reset_search();
      dsState = dsInitDelay;
    }
  }else if (dsState == dsTempConversion)
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
  }else if (dsState == dsInitDelay)
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
  delay(3000);
}



