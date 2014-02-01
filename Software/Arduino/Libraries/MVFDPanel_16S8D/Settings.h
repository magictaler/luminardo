#ifndef MVFDPanel_16S8D_Settings_H
#define MVFDPanel_16S8D_Settings_H

// When will we drop support for the older bug-ridden stuff?
#if defined(ARDUINO) && ARDUINO >=100
#include <Arduino.h>
#else
#include <WProgram.h>
// I am not sure what WProgram.h does not include, so these are here. --xxxajk
#include <pins_arduino.h>
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <stdio.h> // for size_t
#endif

#endif