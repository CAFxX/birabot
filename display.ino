#include "pins.h"

LiquidCrystal lcd(PIN_LCD0, PIN_LCD1, PIN_LCD2, PIN_LCD3, PIN_LCD4, PIN_LCD5);

#define printAt(x, y, data) {               \
  lcd.setCursor((x), (y));                  \
  lcd.print(data);                          \
}

#define printScreen(line0, line1) {         \
  printAt(0, 0, line0);                     \
  printAt(0, 1, line1);                     \
}

#define printfAt(x, y, fmt, ...) {          \
  const int max_len = 17;                   \
  char buf[max_len] = {0};                  \
  snprintf(buf, max_len, fmt, __VA_ARGS__); \
  printAt(x, y, buf);                       \
}

class main_menu;
class program_abort;
class program_menu;
class program_select;
class program_progress;
class program_setup;
class manual_control;
class reset_confirm;

void clear_display() {
  lcd.clear();
  lcd.noBlink();
  lcd.noCursor();
  lcd.noAutoscroll();
}

void setup_display() {
  lcd.begin(16, 2);
  clear_display();
  uxmgr::show<splash_screen>();
  uxmgr::show<main_menu>();
}

class splash_screen : public ux {
  void draw() {
    printScreen(
      "XXXX BIRABOT  ",
      "XXXX " __DATE__
    );
  }
};

class main_menu : public ux {
  void draw() {
    printScreen(
      "A-Manual   Run-B",
      "C-Prog   Reset-D"
    );
  }
  void on_key(char key) {
    switch (key) {
      case 'A': uxmgr::show<manual_control>(); break;
      case 'B': uxmgr::show<program_progress>(); break;
      case 'C': uxmgr::show<program_setup>(); break;
      case 'D': uxmgr::show<reset_confirm>(); break;
    }
  }  
};

/* 
   +----------------+
   | Abort program? |
   |*-Cont   Abort-#|
   +----------------+ 
   Program abort confirmation
*/
class program_abort : public ux {
  
  void draw() {
    printScreen(
      " Abort program? ",
      "*-Abort   Cont-#"
    );
  }
  
  void on_key(char key) {
    switch (key) {
      case '*': uxmgr::show<program_progress>(); break;
      case '#': do_program_abort(); uxmgr::show<main_menu>(); break;
    }
  }
  
  void do_program_abort() {
    // TODO
  }
  
};

class program_menu : public ux {
  void draw() {
  }
  void on_key(char key) {
  }
};

/* 
   +----------------+
   |00°>00° I+ G+ F+|
   |pppppppp 000+000|
   +----------------+ 
   Progress screen shown during program execution
*/
class program_progress : public ux {
  
  void draw() {
    int T1 = 37, s = 725;
    program *prg;
  
    // first line
    printfAt(0, 0, "%02d°%c%02d° I%c G%c F%c", 
      T1, uxmgr::left_arrow, temperature_at(prg, s/60), 
      ignition_on() ? '+' : '-', 
      gasvalve_on() ? '+' : '-', 
      flame_on() ? '+' : '-');
    
    // second line
    printfAt(0, 1, "%8s %03d+03d", 
      program_name(prg), s/60, s%60);
  }
  
  void on_key(char key) {
    switch (key) {
      case '*': uxmgr::show<program_abort>(); break;
      default:  uxmgr::show<program_menu>(); break;
    }
  }
  
};

class manual_control : public ux {
  
  void draw() {
    int T1 = 37, T2 = 40;
  
    // first line
    printfAt(0, 0, "%02d°%c%02d° I%c G%c F%c", 
      T1, uxmgr::left_arrow, T2, 
      ignition_on() ? '+' : '-', 
      gasvalve_on() ? '+' : '-', 
      flame_on() ? '+' : '-');
    
    // second line
    printAt(0, 1, "**** MANUAL ****");
  }
  
  void on_key(char key) {
    switch (key) {
      case '*': uxmgr::show<program_abort>(); break;
      default:  uxmgr::show<program_menu>(); break;
    }
  }
  
};

/*
   +----------------+
   |PPP pppppppp LLL|
   |SS/ss  M DDD TTT|
   +----------------+ 
   Program input screen
   PPP program file id
   ppp program name
   LLL program duration
   SS  current step
   ss  total steps
   M   step mode
   DDD step duration
   TTT step temperature
*/

class program_setup : public ux {
  void draw() {
    program *prg;
    byte file_id, cur_step, num_steps, step_duration, step_temperature, step_mode;
    int program_duration;
    
    // first line
    printfAt(0, 0, "%03d %08s %03d", 
      file_id, program_name(prg), program_duration);
    
    // second line
    printfAt(0, 1, "%02d/%02d  %c %03d %03d", 
      cur_step, num_steps, step_mode, step_duration, step_temperature);
  }
  void on_key(char key) {
  }
};


/* 
   +----------------+
   |DELETE ALL DATA?|
   |*-Delete Abort-#|
   +----------------+ 
   Deep reset confirmation
*/
class reset_confirm : public ux {
  void draw() {
    printScreen(
      "DELETE ALL DATA?",
      "*-Delete Abort-#"
    );
  }
  void on_key(char key) {
    switch (key) {
      case '*': uxmgr::show<main_menu>(); break;
      case '#': do_reset(); uxmgr::show<main_menu>(); break;
    }
  }
  void do_reset() {
    fs.format();
    reset();
  }
};

