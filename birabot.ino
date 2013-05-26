#include <OneWire.h>
#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <FlexiTimer2.h>
#include <Keypad.h>
#include <Time.h>
#include "microfs.h"
#include "uxmgr.h"

static void dumpMemoryStatistics() {
  extern char *__data_start, *__data_end, *__bss_start, *__bss_end, *__heap_start;
  extern int *__brkval;
  Serial.print(F(".data "));
  Serial.print((unsigned)&__data_start);
  Serial.print(' ');
  Serial.print((unsigned)&__data_end);
  Serial.print(' ');
  Serial.println((unsigned)&__data_end - (unsigned)&__data_start);
  Serial.print(F(".bss  "));
  Serial.print((unsigned)&__bss_start);
  Serial.print(' ');
  Serial.print((unsigned)&__bss_end);
  Serial.print(' ');
  Serial.println((unsigned)&__bss_end - (unsigned)&__bss_start);
  Serial.print(F("heap  "));
  Serial.print((unsigned)&__heap_start);
  Serial.print(' ');
  Serial.print((unsigned)__brkval);
  Serial.print(' ');
  Serial.print((unsigned)SP);
  Serial.print(' ');
  Serial.println(freeRam());
}

void setup() {
  Serial.begin(9600);
  dumpMemoryStatistics();
  setup_pins();
  setup_display();
  setup_keypad();
  setup_random();
  setup_fs();
  //setup_temperature();
  setup_flame_sensor();
  setup_safety();
  resume();
}

void loop() {
  // poll the flame sensor
  //poll_flame_sensor();
  // fetch the temperature sensor
  //poll_temperature();
  // scan the keypad
  poll_keypad();
  // draw the UI
  uxmgr::get().draw();
}

