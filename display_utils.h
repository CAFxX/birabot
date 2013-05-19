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

#define printScreen(line0, line1) {         \
  printAt(0, 0, line0);                     \
  printAt(0, 1, line1);                     \
}

#define printScreen_P(line0, line1) {       \
  printAt_P(0, 0, line0);                   \
  printAt_P(0, 1, line1);                   \
}

static void printfAt(byte x, byte y, char *fmt, ...) {          
  const int max_len = 17;                   
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

static void __lcdCreateCharPGM(byte id, byte *data, boolean invert) {
  if (invert) lcd_char_invert(data);
  lcd.createChar(id, data);
}

#define lcdCreateCharPGM(id, data, invert) {\
  __blob_PGM(data, 8);                      \
  __lcdCreateCharPGM(id, __buf__, invert);  \
}

#endif // DISPLAY_UTILS
