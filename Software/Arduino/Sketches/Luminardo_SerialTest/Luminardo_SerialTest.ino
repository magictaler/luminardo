/*
  Luminardo Serial Test 
 
  This example code is in the public domain.
 */

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 57600 bits per second:
  Serial.begin(57600);
  Serial.println(F("Luminardo Serial Test"));
}

// the loop routine runs over and over again forever:
void loop() {
  Serial.println(F("Test"));
  delay(200);
}



