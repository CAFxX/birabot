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

#define writeAt(x, y, b) {                  \
  lcd.setCursor((x), (y));                  \
  lcd.write((byte)b);                       \
}

class main_menu;
class program_abort;
class program_menu;
class program_select;
class program_progress;
class program_setup;
class program_list;
class manual_control;
class reset_confirm;
class splash_screen;

#include "custom_chars.h"

void clear_display() {
  lcd.clear();
  lcd.noBlink();
  lcd.noCursor();
  lcd.noAutoscroll();
}

void setup_display() {
  lcd.begin(16, 2);
  lcd.createChar(0, logo00);  
  lcd.createChar(1, logo01);  
  lcd.createChar(2, logo02);  
  lcd.createChar(3, logo03);  
  lcd.createChar(4, logo10);  
  lcd.createChar(5, logo11);  
  lcd.createChar(6, logo12);  
  lcd.createChar(7, logo13);  
  uxmgr::show<splash_screen>();
}

class splash_screen : public ux {
  void draw() {
    clear_display();
    writeAt(0, 0, 0);
    writeAt(1, 0, 1);
    writeAt(2, 0, 2);
    writeAt(3, 0, 3);
    writeAt(0, 1, 4);
    writeAt(1, 1, 5);
    writeAt(2, 1, 6);
    writeAt(3, 1, 7);
    printAt(5, 0, "BIRABOT");
    printAt(5, 1, __DATE__);
  }
  void on_key(char) {
    uxmgr::show<main_menu>();
  };
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
      case 'C': uxmgr::show<program_menu>(); break;
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

class program_list : public ux {
  byte file_id;
  public:
  program_list() {
    file_id = 0;
  }
  void draw() {
    program *prg;
    printfAt(0, 0, "%03d %08s %03d", 
      file_id, program_name(prg), program_duration);
    printAt(0, 1, "*-Back    Menu-#");
  }
  void on_key(char key) {
    switch (key) {
      case 'A': file_id--; break;
      case 'B': file_id -= 10; break;
      case 'C': file_id += 10; break;
      case 'D': file_id++; break;
      case '0': case '1': case '2': case '3': case '4': 
      case '5': case '6': case '7': case '8': case '9':
        break;
      case '#': uxmgr::back(file_id); break;
      case '*': uxmgr::back(); break;
    }
  }

};

/* 
   +----------------+
   |A-Create  Edit-B|
   |C-Copy  Delete-D|
   +----------------+ 
   Program operations
*/
class program_menu : public ux {
  char last_key;
  public:
  program_menu() {
    last_key = 0;
  }
  void draw() {
    printScreen(
      "A-Create  Edit-B",
      "C-Copy  Delete-D"
    );
  }
  void on_key(char key) {
    last_key = key;
    switch (key) {
      case 'A': uxmgr::show<program_list>(this); break;
      case 'B': uxmgr::show<program_list>(this); break;
      case 'C': uxmgr::show<program_list>(this); break;
      case 'D': uxmgr::show<program_list>(this); break;
      case '*': uxmgr::back(); break;
    }
  }
  void on_back(int retVal) {
    switch (last_key) {
      case 'A': uxmgr::show<program_setup>(this, retVal); break;
      case 'B': uxmgr::show<program_setup>(this, retVal); break;
      case 'C': break;
      case 'D': fs.remove(retVal); break;
    }
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
    int s = 725;
    program *prg;
  
    // first line
    printfAt(0, 0, "%02d°\x7e%02d° I%c G%c F%c", 
      get_temperature(), temperature_at(prg, s/60), 
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
    }
  }
  
};

/* 
   +----------------+
   |00°>00° I+ G+ F+|
   |*-Stop  Temp-0-9|
   +----------------+ 
   Manual control screen
*/
class manual_control : public ux {
  
  byte temp_set;
  byte temp_prev;
  boolean temp_valid;
  
  public:
  manual_control() {
    temp_set = 0;
    temp_valid = false;
  }
  
  void draw() {
    // first line
    printfAt(0, 0, "%02d°\x7E%02d° I%c G%c F%c", 
      get_temperature(), temp_set, 
      ignition_on() ? '+' : '-', 
      gasvalve_on() ? '+' : '-', 
      flame_on() ? '+' : '-');
    
    if (temp_valid) {
      printAt(0, 1, "*-Stop  Temp-0-9");
    } else {
      printAt(0, 1, "*-Cancel   Set-#");
      lcd.setCursor(0, 5);
      lcd.cursor();
    }
  }
  
  void on_key(char key) {
    switch (key) {
      case '0': case '1': case '2': case '3': case '4': 
      case '5': case '6': case '7': case '8': case '9':
        if (temp_valid) {
          temp_valid = false;
          temp_prev = temp_set;
        }
        temp_set %= 10;
        temp_set *= 10;
        temp_set += key - '0';
        break;
      case '#':
        temp_valid = true;
        // TODO: send the new temp to the engine
        break;
      case '*': 
        if (temp_valid) {
          // TODO: send temp=0 to the engine
          uxmgr::show<main_menu>(); 
        } else {
          temp_valid = true;
          temp_set = temp_prev;
        }
        break;
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
   LLL program duration (minutes)
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

