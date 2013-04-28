#include "pins.h"

void setup_pins() {
  // set the ADC reference to 1.1V
  analogReference(INTERNAL);
  // set the mode for all pins
  // LCD pins
  pinMode(PIN_LCD0, OUTPUT);
  pinMode(PIN_LCD1, OUTPUT);
  pinMode(PIN_LCD2, OUTPUT);
  pinMode(PIN_LCD3, OUTPUT);
  pinMode(PIN_LCD4, OUTPUT);
  pinMode(PIN_LCD5, OUTPUT);
  // keypad pins
  pinMode(PIN_KEYPAD_COL0, INPUT);
  pinMode(PIN_KEYPAD_COL1, INPUT);
  pinMode(PIN_KEYPAD_COL2, INPUT);
  pinMode(PIN_KEYPAD_COL3, INPUT);
  pinMode(PIN_KEYPAD_ROW0, INPUT);
  pinMode(PIN_KEYPAD_ROW1, INPUT);
  pinMode(PIN_KEYPAD_ROW2, INPUT);
  pinMode(PIN_KEYPAD_ROW3, INPUT);
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



