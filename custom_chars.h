byte logo00[] = {
  B00001,
  B00010,
  B01110,
  B10001,
  B10000,
  B01111,
  B01101,
  B01101
};
byte logo01[] = {
  B11000,
  B01110,
  B10001,
  B00001,
  B00001,
  B11110,
  B10111,
  B10111
};
byte logo02[] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B10000,
  B11000
};
byte logo03[] = {
  B11111,
  B10000,
  B00111,
  B00101,
  B00000,
  B00111,
  B00101,
  B11111
};
byte logo10[] = {
  B01101,
  B01101,
  B01101,
  B01101,
  B01101,
  B01101,
  B01111,
  B00111
};
byte logo11[] = {
  B10110,
  B10110,
  B10110,
  B10110,
  B10110,
  B10111,
  B11111,
  B11100
};
byte logo12[] = {
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B10000,
  B00000
};
byte logo13[] = {
  B00100,
  B00111,
  B00000,
  B00111,
  B00000,
  B00111,
  B00101,
  B11111
};

byte sym_ignition_off[] = {
  B00000,
  B01110,
  B00100,
  B00100,
  B00100,
  B00100,
  B01110,
  B00000
};
byte sym_ignition_on[] = {
  ~sym_ignition_off[0],
  ~sym_ignition_off[1],
  ~sym_ignition_off[2],
  ~sym_ignition_off[3],
  ~sym_ignition_off[4],
  ~sym_ignition_off[5],
  ~sym_ignition_off[6],
  ~sym_ignition_off[7]
};

byte sym_gasvalve_off[] = {
  B00000,
  B01110,
  B01000,
  B01000,
  B01010,
  B01010,
  B01110,
  B00000
};
byte sym_gasvalve_on[] = {
  ~sym_gasvalve_off[0],
  ~sym_gasvalve_off[1],
  ~sym_gasvalve_off[2],
  ~sym_gasvalve_off[3],
  ~sym_gasvalve_off[4],
  ~sym_gasvalve_off[5],
  ~sym_gasvalve_off[6],
  ~sym_gasvalve_off[7]
};

byte sym_flame_off[] = {
  B00000,
  B01110,
  B01000,
  B01000,
  B01110,
  B01000,
  B01000,
  B00000
};
byte sym_flame_on[] = {
  ~sym_flame_off[0],
  ~sym_flame_off[1],
  ~sym_flame_off[2],
  ~sym_flame_off[3],
  ~sym_flame_off[4],
  ~sym_flame_off[5],
  ~sym_flame_off[6],
  ~sym_flame_off[7]
};
