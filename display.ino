#include "pins.h"

LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_ENABLE, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);

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
    show<main_menu>();
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
      case 'A': next<manual_control>(); break;
      case 'B': next<program_progress>(); break;
      case 'C': next<program_menu>(); break;
      case 'D': show<reset_confirm>(); break;
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
      case '*': back(); break;
      case '#': do_program_abort(); show<main_menu>(); break;
    }
  }
  
  void do_program_abort() {
    // TODO
  }
  
};

class program_list : public ux {
  byte file_id;
  bool typing;
  public:
  program_list() {
    file_id = 0;
    typing = false;
  }
  void draw() {
    program *prg;
    printfAt(0, 0, "%03d %08s %03d", 
      file_id, program_name(prg), program_duration);
    printAt(0, 1, "*-Back    Menu-#");
  }
  void on_key(char key) {
    switch (key) {
      case 'A': typing = false; file_id--; break;
      case 'B': typing = false; file_id -= 10; break;
      case 'C': typing = false; file_id += 10; break;
      case 'D': typing = false; file_id++; break;
      case '0': case '1': case '2': case '3': case '4': 
      case '5': case '6': case '7': case '8': case '9': {
        int new_file_id;
        if (!typing) {
          new_file_id = 0;
          typing = true;
        } else {
          new_file_id = file_id;
          new_file_id *= 10;
        }
        new_file_id += key - '0';
        if (new_file_id > 255) {
          new_file_id %= 100;
        }
        file_id = new_file_id;
        break;
      }
      case '#': back(file_id); break;
      case '*': back(); break;
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
  byte copy_source_file_id;
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
      case 'A': 
      case 'B': 
      case 'C': 
      case 'D': next<program_list>(); break;
      case '*': back(); break;
    }
  }
  void on_back(int retVal) {
    switch (last_key) {
      case 'A': 
      case 'B': next<program_setup>(retVal); break;
      case 'C': 
        last_key = 'c'; 
        copy_source_file_id = retVal;
        show<program_list>(); 
        break;
      case 'c': {
        microfsfile src = fs.open(copy_source_file_id);
        microfsfile dst = fs.create(src.get_size(), retVal);
        for (int i=0; i<src.get_size(); i++) {
          dst.write_byte(i, src.read_byte(i));
        }
      }
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
      case '*': next<program_abort>(); break;
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
          back(); 
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
      case '*': back(); break;
      case '#': do_reset(); break;
    }
  }
  void do_reset() {
    fs.format();
    reset();
    while (true);
  }
};

class microfs_tool : public ux {
  byte row;
  boolean check;
  size_t used;
  size_t free;
  size_t total;
  byte files;
  byte max_free_chunk;
  microfs_tool() {
    row = 0;
    check = fs.check_disk();
    used = fs.used();
    free = fs.free();
    total = fs.total();
    files = fs.files();
    max_free_chunk = fs.max_free_chunk();
  }
  void draw() {
    switch (row) {          
      case 0: printfAt(0, 0, "Check %10s", check ? "OK" : "ERROR"); break;
      case 1: printfAt(0, 0, "Used %11d", used); break;
      case 2: printfAt(0, 0, "Free %11d", free); break;
      case 3: printfAt(0, 0, "Total %10d", total); break;
      case 4: printfAt(0, 0, "Files %10d", files); break;
      case 5: printfAt(0, 0, "Max chunk %6d", max_free_chunk); break;
    }
    printAt(0, 1, "*-Back          ");
  }
  void on_key(char key) {
    switch (key) {
      case 'A': row = (row-1) % 6; break;
      case 'D': row = (row+1) % 6; break;
      case '*': back(); break;
    }
  }
};

