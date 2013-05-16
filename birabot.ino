#include <OneWire.h>
#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <FlexiTimer2.h>
#include <Keypad.h>
#include "microfs.h"
#include "uxmgr.h"

void setup() {
  noInterrupts();
  setup_pins();
  setup_random();
  setup_display();
  setup_keypad();
  setup_fs();
  setup_temperature();
  interrupts();
  setup_safety();
  resume();
}

size_t seconds = 0;

static void resume() {
  microfsfile resumefile = fs.open(1);
  if (!resumefile.is_valid() || resumefile.get_size() != 3) {
    fs.remove(1);
    return;
  }
  byte buf[3] = {0};
  resumefile.read_bytes(0, buf, sizeof(buf));
  microfsfile programfile = fs.open(buf[0]);
  if (!programfile.is_valid()) {
    fs.remove(1);
    return;
  }
  seconds = buf[1];
  seconds <<= 8;
  seconds |= buf[2];
}

void loop() {
  uxmgr::get().draw();
}

