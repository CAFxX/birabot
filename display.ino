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
class program_save;
class program_list;
class manual_control;
class reset_confirm;
class splash_screen;
class microfs_tool;

#include "custom_chars.h"

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
}

class splash_screen : public ux {
  public:
  splash_screen() {
    lcd.createChar(0, logo00);  
    lcd.createChar(1, logo01);  
    lcd.createChar(2, logo02);  
    lcd.createChar(3, logo03);  
    lcd.createChar(4, logo10);  
    lcd.createChar(5, logo11);  
    lcd.createChar(6, logo12);  
    lcd.createChar(7, logo13);  
  }
  void draw() {
    printAt(5, 0, "BIRABOT");
    printAt(5, 1, __DATE__);
    // draw the logo
    writeAt(0, 0, 0);
    writeAt(1, 0, 1);
    writeAt(2, 0, 2);
    writeAt(3, 0, 3);
    writeAt(0, 1, 4);
    writeAt(1, 1, 5);
    writeAt(2, 1, 6);
    writeAt(3, 1, 7);
  }
  void on_key(char) {
    show<main_menu>();
  };
};

class main_menu : public ux {
  void draw() {
    printScreen(
      "A-Manual   Run-B",
      "C-Prog   Tools-D"
    );
  }
  void on_key(char key) {
    switch (key) {
      case 'A': next<manual_control>(); break;
      case 'B': next<program_progress>(); break;
      case 'C': next<program_menu>(); break;
      case 'D': next<microfs_tool>(); break;
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
   |00°>00°  a i g f|
   |pppppppp 000+000|
   +----------------+ 
   Progress screen shown during program execution
*/
class program_progress : public ux {
  public:
  program_progress() {
    lcd.createChar(0, sym_ignition_off);  
    lcd.createChar(1, sym_ignition_on);  
    lcd.createChar(2, sym_gasvalve_off);  
    lcd.createChar(3, sym_gasvalve_on);  
    lcd.createChar(4, sym_flame_off);  
    lcd.createChar(5, sym_flame_on);  
  }
  void draw() {
    int s = 725;
    program *prg;
  
    // first line
    printfAt(0, 0, "%02d°\x7e%02d°    %c %c %c", 
      get_temperature(), temperature_at(prg, s/60), 
      ignition_on() ? 1 : 0, 
      gasvalve_on() ? 3 : 2, 
      flame_on() ? 5 : 4);
    
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
   |00°>00°  a i g f|
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
    lcd.createChar(0, sym_ignition_off);  
    lcd.createChar(1, sym_ignition_on);  
    lcd.createChar(2, sym_gasvalve_off);  
    lcd.createChar(3, sym_gasvalve_on);  
    lcd.createChar(4, sym_flame_off);  
    lcd.createChar(5, sym_flame_on);  
  }
  
  void draw() {
    // first line
    printfAt(0, 0, "%02d°\x7e%02d°    %c %c %c", 
      get_temperature(), temp_set, 
      ignition_on() ? 1 : 0, 
      gasvalve_on() ? 3 : 2, 
      flame_on() ? 5 : 4);
    
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
   |SS/ss   M DDD TT|
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
  byte file_id;
  byte field;
  byte row;
  byte rows;
  program *prg;
  
  ux_input_numeric<2, 1> type_mode;
  ux_input_numeric<256, 100> type_duration;
  ux_input_numeric<100, 100> type_temp;
  
  public:
  program_setup() {
    row = 0;
    rows = 0;
    field = 0;
    file_id = 0;
    prg = NULL;
  }
  void on_init(int param) {
    file_id = param;
  }
  void draw() {
    byte file_id, cur_step, num_steps, step_duration, step_temperature, step_mode;
    int program_duration;
    
    // first line
    printfAt(0, 0, "%03d %08s %03d", 
      file_id, program_name(prg), program_duration);
    
    // second line
    printfAt(0, 1, "%02d/%02d   %c %03d %02d", 
      row, rows+1, type_mode() ? 'C' : 'L', type_duration(), type_temp());
    
    switch (field) {
      case 0: lcd.setCursor(7, 1); break;
      case 1: lcd.setCursor(11, 1); break;
      case 2: lcd.setCursor(15, 1); break;
    }
    lcd.cursor();
  }
  void on_key(char key) {
    switch (key) {
      case 'A':
        row = ( row - 1 + rows+1 ) % ( rows+1 );
        type_mode = get_step_type(prg, row);
        type_duration = get_step_duration(prg, row);
        type_temp = get_step_temperature(prg, row); 
        break;
      case 'B':
        field = ( field - 1 + 3 ) % 3;
        break;
      case 'C':
        field = ( field + 1 + 3 ) % 3;
        break;
      case 'D':
        row = ( row + 1 + rows+1 ) % ( rows+1 );
        type_mode = get_step_type(prg, row);
        type_duration = get_step_duration(prg, row);
        type_temp = get_step_temperature(prg, row); 
        break;
      case '0': case '1': case '2': case '3': case '4': 
      case '5': case '6': case '7': case '8': case '9': 
        switch (field) {
          case 0: 
            type_mode.on_key(key); 
            set_step_type(prg, row, type_mode()); 
            break;
          case 1: 
            type_duration.on_key(key); 
            set_step_duration(prg, row, type_duration()); 
            break;
          case 2: 
            type_temp.on_key(key); 
            set_step_temperature(prg, row, type_temp()); 
            break;
        }
        break;
      case '*': next<program_save>();
    }
  }
  void on_back(int retVal) {
    if (retVal) {
      // TODO: save changes
    }
    back();
  }
};

class program_save : public ux {
  void draw() {
    printScreen(
      "  Save changes? ",
      "*-Discard Save-#"
    );    
  }
  void on_key(char key) {
    switch (key) {
      case '*': back(0); break;
      case '#': back(1); break;
    }
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
      "*-Abort Delete-#"
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
  public:
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
      case 0: printfAt(0, 0, "Check     %6s", check ? "OK" : "ERROR"); break;
      case 1: printfAt(0, 0, "Used      %6d", used); break;
      case 2: printfAt(0, 0, "Free      %6d", free); break;
      case 3: printfAt(0, 0, "Total     %6d", total); break;
      case 4: printfAt(0, 0, "Files     %6d", files); break;
      case 5: printfAt(0, 0, "Max chunk %6d", max_free_chunk); break;
    }
    if (row == 0) {
      printAt(0, 1, "*-Back  Format-#");
    } else {
      printAt(0, 1, "*-Back          ");
    }
  }
  void on_key(char key) {
    switch (key) {
      case 'A': row = (row-1) % 6; break;
      case 'D': row = (row+1) % 6; break;
      case '#': if (row == 0) next<reset_confirm>(); break;
      case '*': back(); break;
    }
  }
};

