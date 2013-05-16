PROGMEM byte logo00[] = {
  B00001,
  B00010,
  B01110,
  B10001,
  B10000,
  B01111,
  B01101,
  B01101
};
PROGMEM byte logo01[] = {
  B11000,
  B01110,
  B10001,
  B00001,
  B00001,
  B11110,
  B10111,
  B10111
};
PROGMEM byte logo02[] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B10000,
  B11000
};
PROGMEM byte logo03[] = {
  B11111,
  B10000,
  B00111,
  B00101,
  B00000,
  B00111,
  B00101,
  B11111
};
PROGMEM byte logo10[] = {
  B01101,
  B01101,
  B01101,
  B01101,
  B01101,
  B01101,
  B01111,
  B00111
};
PROGMEM byte logo11[] = {
  B10110,
  B10110,
  B10110,
  B10110,
  B10110,
  B10111,
  B11111,
  B11100
};
PROGMEM byte logo12[] = {
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B10000,
  B00000
};
PROGMEM byte logo13[] = {
  B00100,
  B00111,
  B00000,
  B00111,
  B00000,
  B00111,
  B00101,
  B11111
};

#define lcd_char_invert(name) { \
  name[0] = (~name[0]) & B11111; \
  name[1] = (~name[1]) & B11111; \
  name[2] = (~name[2]) & B11111; \
  name[3] = (~name[3]) & B11111; \
  name[4] = (~name[4]) & B11111; \
  name[5] = (~name[5]) & B11111; \
  name[6] = (~name[6]) & B11111; \
  name[7] = (~name[7]) & B11111; \
}

PROGMEM byte sym_ignition_off[] = {
  B00000,
  B01110,
  B00100,
  B00100,
  B00100,
  B00100,
  B01110,
  B00000
};

PROGMEM byte sym_gasvalve_off[] = {
  B00000,
  B01110,
  B01000,
  B01000,
  B01010,
  B01010,
  B01110,
  B00000
};

PROGMEM byte sym_flame_off[] = {
  B00000,
  B01110,
  B01000,
  B01000,
  B01110,
  B01000,
  B01000,
  B00000
};

PROGMEM byte sym_alarm_off[] = {
  B00000,
  B01110,
  B01010,
  B01010,
  B01110,
  B01010,
  B01010,
  B00000
};

