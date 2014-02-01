/*
  Luminardo PIR Sensor Test 
 
  This example code is in the public domain.
 */
 
#include <Board.h>

volatile uint8_t pirEvent;

// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(57600);
  Serial.println(F("Luminardo PIR Sensor Test"));
  
  pinMode(PIR_PIN, INPUT);
  
//  PCICR |= (1 << PCIE0);
//  PCMSK0 |= (1 << PCINT1);
  EICRA |= (1 << ISC21) | (1 << ISC20);
  EIMSK |= (1 << INT2);
  
  digitalWrite(PIR_PIN, 0);
  pirEvent = false;
}

//SIGNAL(PCINT1_vect)
SIGNAL(INT2_vect)
{
  cli();  
  pirEvent = true;
  sei();
}

// the loop routine runs over and over again forever:
void loop() {
  if (digitalRead(PIR_PIN))
    Serial.println(F("PIR sensor state is HIGH"));
  else
    Serial.println(F("PIR sensor state is LOW"));
  if (pirEvent)
  {
    pirEvent = false;  
    Serial.println(F("PIR sensor detected motion!"));
  }

  delay(5000);
}



