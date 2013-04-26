#include "pins.h"

LiquidCrystal lcd(PIN_LCD0, PIN_LCD1, PIN_LCD2, PIN_LCD3, PIN_LCD4, PIN_LCD5);

#define printAt(x, y, data) {               \
  lcd.setCursor((x), (y));                  \
  lcd.print(data);                          \
}
#define printfAt(x, y, fmt, ...) {          \
  const int max_len = 17;                   \
  char buf[max_len] = {0};                  \
  snprintf(buf, max_len, fmt, __VA_ARGS__); \
  printAt(x, y, buf);                       \
}

void setup_display() {
  lcd.begin(16, 2);
  clear_display();
  printAt(0, 0, "BIRABOT");
  printAt(0, 1, __DATE__ " " __TIME__);
}

void clear_display() {
  lcd.clear();
  lcd.noBlink();
  lcd.noCursor();
  lcd.noAutoscroll();
}

#define LEFT_ARROW B01111110

/* +----------------+
   |00°>00° I+ G+ F+|
   |ppppppppp 000:00|
   +----------------+ */
void screen_status() {
  int T1 = 37, s = 725;
  char P[] = "pippo";
  program *prg;

  clear_display();
  printfAt(0, 0, "%02d°%c%02d° I%c G%c F%c", 
    T1, LEFT_ARROW, temperature_at(prg, s/60), 
    ignition_on() ? '+' : '-', 
    gasvalve_on() ? '+' : '-', 
    flame_on() ? '+' : '-');
  
  printfAt(0, 1, "%9s %03d:02d", program_name(prg), s/60, s%60);
}
