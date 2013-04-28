#include <OneWire.h>
#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <FlexiTimer2.h>
#include <Keypad.h>
#include "microfs.h"

void setup() {
  noInterrupts();
  setup_pins();
  setup_random();
  setup_display();
  setup_keypad();
  setup_fs();
  interrupts();
  setup_safety();
  resume();
}

void resume() {
  microfsfile resumefile = open_file(1);
  byte buf[3] = {0};
  resumefile.read_bytes(0, buf, sizeof(buf));
  open_file(buf[0]);
  
}

void loop() {
  screen_status();
}

