#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <FlexiTimer2.h>
#include <Keypad.h>

void setup() {
  setup_pins();
  setup_display();
  setup_keypad();
  setup_safety();
}

void loop() {
  screen_status();
}
