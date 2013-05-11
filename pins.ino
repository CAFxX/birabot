#include "pins.h"

static void setup_pins() {
  // set the ADC reference to 1.1V
  analogReference(INTERNAL);
  // set the mode for all pins
  // keypad pins are configured internally by the Keypad library
  // LCD pins
  pinMode(PIN_LCD0, OUTPUT);
  pinMode(PIN_LCD1, OUTPUT);
  pinMode(PIN_LCD2, OUTPUT);
  pinMode(PIN_LCD3, OUTPUT);
  pinMode(PIN_LCD4, OUTPUT);
  pinMode(PIN_LCD5, OUTPUT);
  // temperature sensor pin (digital)
  pinMode(PIN_TEMP_SENS, INPUT);
  // flame sensor (thermocouple) pin (analog)
  pinMode(PIN_FLAME_SENS, INPUT);
  // ignition control pin
  pinMode(PIN_IGNITION, OUTPUT);
  // gas valve control pin
  pinMode(PIN_GASVALVE, OUTPUT);
  // unconnected pin, used as entropy source (analog)
  pinMode(PIN_UNCONNECTED, INPUT);
}



