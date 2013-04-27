#include "pins.h"

void setup_pins() {
  pinMode(PIN_LCD0, OUTPUT);
  pinMode(PIN_LCD1, OUTPUT);
  pinMode(PIN_LCD2, OUTPUT);
  pinMode(PIN_LCD3, OUTPUT);
  pinMode(PIN_LCD4, OUTPUT);
  pinMode(PIN_LCD5, OUTPUT);
  pinMode(PIN_KEYPAD_COL0, INPUT);
  pinMode(PIN_KEYPAD_COL1, INPUT);
  pinMode(PIN_KEYPAD_COL2, INPUT);
  pinMode(PIN_KEYPAD_COL3, INPUT);
  pinMode(PIN_KEYPAD_ROW0, INPUT);
  pinMode(PIN_KEYPAD_ROW1, INPUT);
  pinMode(PIN_KEYPAD_ROW2, INPUT);
  pinMode(PIN_KEYPAD_ROW3, INPUT);
  pinMode(PIN_TEMP_SENS, INPUT);
  pinMode(PIN_FLAME_SENS, INPUT);
  pinMode(PIN_IGNITION, OUTPUT);
  pinMode(PIN_GASVALVE, OUTPUT);
  pinMode(PIN_UNCONNECTED, INPUT);
}



