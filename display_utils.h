#ifndef DISPLAY_UTILS
#define DISPLAY_UTILS

extern LiquidCrystal lcd;

#define __string_PGM(data)                  \
  char __buf__[sizeof(data)];               \
  strcpy_P(__buf__, PSTR(data));             

#define __blob_PGM(blobname, size)          \
  byte __buf__[size];                       \
  memcpy_P(__buf__, blobname, size);         

static void printAt(byte x, byte y, char *data) {
  lcd.setCursor(x, y);
  lcd.print(data);
}

static void writeAt(byte x, byte y, byte data) {
  lcd.setCursor(x, y);
  lcd.write(data);
}

#define printAt_P(x, y, data) {             \
  __string_PGM(data);                       \
  printAt(x, y, __buf__);                   \
}

#define printScreen_P(line0, line1) { \
  printAt_P(0, 0, line0);                           \
  printAt_P(0, 1, line1);                           \
}

#define printScreen_P4(line0, line1, line2, line3) { \
  printAt_P(0, 0, line0);                           \
  printAt_P(0, 1, line1);                           \
  printAt_P(0, 2, line2);                           \
  printAt_P(0, 3, line3);                           \
}

#define printScreen_P2(line1, line2) { \
  clearLine(0);                                     \
  printAt_P(0, 1, line1);                           \
  printAt_P(0, 2, line2);                           \
  clearLine(3);                                     \
}

static void printfAt(byte x, byte y, char *fmt, ...) {          
  const int max_len = 20+1;                   
  char buf[max_len];        
  va_list args;
  va_start(args, fmt);  
  vsnprintf(buf, max_len, fmt, args); 
  va_end(args);
  printAt(x, y, buf);                       
}

#define printfAt_P(x, y, fmt, ...) {        \
  __string_PGM(fmt);                        \
  printfAt(x, y, __buf__, __VA_ARGS__);     \
}

#define printLineAt_P(x, y, data) {    \
  const int max_len = 20+1;            \    
  char buf[max_len];                   \
  __string_PGM(data);                  \
  memset(buf, ' ', max_len);           \
  strncpy(buf+x, __buf__, max_len-x);  \
  printAt(0, y, buf);                  \
}
  
static void __lcdCreateCharPGM(byte id, byte *data, boolean invert) {
  if (invert) lcd_char_invert(data);
  lcd.createChar(id, data);
}

#define lcdCreateCharPGM(id, data, invert) {\
  __blob_PGM(data, 8);                      \
  __lcdCreateCharPGM(id, __buf__, invert);  \
}

static void loadLogo() {
  lcdCreateCharPGM(0, logo00, false);  
  lcdCreateCharPGM(1, logo01, false);  
  lcdCreateCharPGM(2, logo02, false);  
  lcdCreateCharPGM(3, logo03, false);  
  lcdCreateCharPGM(4, logo10, false);  
  lcdCreateCharPGM(5, logo11, false);  
  lcdCreateCharPGM(6, logo12, false);  
  lcdCreateCharPGM(7, logo13, false);  
}

static void loadSymbols() {
  lcdCreateCharPGM(0, sym_ignition_off, false);  
  lcdCreateCharPGM(1, sym_ignition_off, true);  
  lcdCreateCharPGM(2, sym_gasvalve_off, false);  
  lcdCreateCharPGM(3, sym_gasvalve_off, true);  
  lcdCreateCharPGM(4, sym_flame_off, false);  
  lcdCreateCharPGM(5, sym_flame_off, true);  
  lcdCreateCharPGM(6, sym_alarm_off, false);  
  lcdCreateCharPGM(7, sym_alarm_off, true);  
}

static void clearLine(int r) {
  for (int c=0; c<20; c++)
    writeAt(c, r, ' ');
}

static void drawLogo(int c, int r, boolean extended=true) {
    writeAt(c+0, r+0, 0);
    writeAt(c+1, r+0, 1);
    writeAt(c+2, r+0, 2);
    if (extended)
      writeAt(c+3, r+0, 3);
    writeAt(c+0, r+1, 4);
    writeAt(c+1, r+1, 5);
    writeAt(c+2, r+1, 6);
    if (extended)
      writeAt(c+3, r+1, 7);
}

#endif // DISPLAY_UTILS
