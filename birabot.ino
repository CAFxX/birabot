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

size_t seconds = 0;

void resume() {
  microfsfile resumefile = open_file(1);
  if (!resumefile.is_valid())
    return;
  byte buf[3] = {0};
  resumefile.read_bytes(0, buf, sizeof(buf));
  microfsfile programfile = open_file(buf[0]);
  if (!programfile.is_valid()) {
    fs.remove(1);
    return;
  }
  seconds = buf[1];
  seconds <<= 8;
  seconds |= buf[2];
}

void loop() {
  screen_status();
}

