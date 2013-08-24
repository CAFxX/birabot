#include <OneWire.h>
#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <FlexiTimer2.h>
#include <Keypad.h>
#include <Time.h>
#include "microfs.h"
#include "uxmgr.h"

void setup() {
  Serial.begin(9600);
  dumpMemoryStatistics();
  setup_pins();
  setup_display();
  setup_keypad();
  setup_random();
  setup_fs();
  setup_temperature();
  setup_flame_sensor();
  setup_safety();
  resume();
}

void loop() {
  // poll the flame sensor
  poll_flame_sensor();
  // fetch the temperature sensor
  poll_temperature();
  // scan the keypad
  poll_keypad();
  // draw the UI
  uxmgr::get().draw();
}

