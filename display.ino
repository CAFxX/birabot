#include "pins.h"
#include "custom_chars.h"
#include "Program.h"
#include "panic.h"
#include "display_utils.h"

LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_ENABLE, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);

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

static void setup_display() {
  lcd.begin(16, 2);
  clear_display();
  uxmgr::get().show<splash_screen>();
}

static void clear_display() {
  lcd.clear();
  lcd.noBlink();
  lcd.noCursor();
  lcd.noAutoscroll();
}

class splash_screen : public ux {
  void on_show() {
    lcdCreateCharPGM(0, logo00, false);  
    lcdCreateCharPGM(1, logo01, false);  
    lcdCreateCharPGM(2, logo02, false);  
    lcdCreateCharPGM(3, logo03, false);  
    lcdCreateCharPGM(4, logo10, false);  
    lcdCreateCharPGM(5, logo11, false);  
    lcdCreateCharPGM(6, logo12, false);  
    lcdCreateCharPGM(7, logo13, false);  
  }
  void draw() {
    printAt_P(5, 0, "BIRABOT");
    printAt_P(5, 1, __DATE__);
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
    printScreen_P(
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
    printScreen_P(
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
  Program prg;
  public:
  program_list() {
    prg = file_id = 0;
    typing = false;
  }
  void draw() {
    printfAt_P(0, 0, "%03d %08s %03d", file_id, "", prg.duration());
    printAt_P(0, 1, "*-Back  Select-#");
  }
  void on_key(char key) {
    switch (key) {
      case 'A': typing = false; prg = file_id -=  1; break;
      case 'B': typing = false; prg = file_id -= 10; break;
      case 'C': typing = false; prg = file_id += 10; break;
      case 'D': typing = false; prg = file_id +=  1; break;
      case '0': case '1': case '2': case '3': case '4': 
      case '5': case '6': case '7': case '8': case '9': {
        ux_input_numeric<256, 100> new_file_id = typing ? file_id : 0;
        typing = true;
        new_file_id.on_key(key);
        prg = file_id = new_file_id;
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
    printScreen_P(
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
  void on_show() {
    lcdCreateCharPGM(0, sym_ignition_off, false);  
    lcdCreateCharPGM(1, sym_ignition_off, true);  
    lcdCreateCharPGM(2, sym_gasvalve_off, false);  
    lcdCreateCharPGM(3, sym_gasvalve_off, true);  
    lcdCreateCharPGM(4, sym_flame_off, false);  
    lcdCreateCharPGM(5, sym_flame_off, true);  
  }
  void draw() {
    int s = 725;

    // first line
    printfAt_P(0, 0, "%02d°\x7e%02d°    %c %c %c", 
      get_temperature(), get_temperature_target(), 
      ignition_on() ? 1 : 0, 
      gasvalve_on() ? 3 : 2, 
      flame_on() ? 5 : 4);
    
    // second line
    printfAt_P(0, 1, "%8s %03d+03d", 
      "", s/60, s%60);
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
  
  byte temp_prev;
  boolean temp_valid;
  
  ux_input_numeric<100, 100> temp_set;
  
  public:
  manual_control() {
    temp_set = 0;
    temp_valid = false;
  }

  void on_show() {
    lcdCreateCharPGM(0, sym_ignition_off, false);  
    lcdCreateCharPGM(1, sym_ignition_off, true);  
    lcdCreateCharPGM(2, sym_gasvalve_off, false);  
    lcdCreateCharPGM(3, sym_gasvalve_off, true);  
    lcdCreateCharPGM(4, sym_flame_off, false);  
    lcdCreateCharPGM(5, sym_flame_off, true);  
  }
  
  void draw() {
    // first line
    printfAt_P(0, 0, "%02d°\x7e%02d°    %c %c %c", 
      get_temperature(), get_temperature_target(), 
      ignition_on() ? 1 : 0, 
      gasvalve_on() ? 3 : 2, 
      flame_on() ? 5 : 4);
    
    if (temp_valid) {
      printAt_P(0, 1, "*-Stop  Temp-0-9");
    } else {
      printAt_P(0, 1, "*-Cancel   Set-#");
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
          temp_prev = temp_set();
        }
        temp_set.on_key(key);
        break;
      case '#':
        temp_valid = true;
        set_temperature_target(temp_set);
        break;
      case '*': 
        if (temp_valid) {
          set_temperature_target(0);
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
  Program prg;
  
  ux_input_numeric<2, 1> type_mode;
  ux_input_numeric<256, 100> type_duration;
  ux_input_numeric<100, 100> type_temp;
  
  public:
  program_setup() {
    row = 0;
    rows = 0;
    field = 0;
    prg = file_id = 0;
  }
  void on_init(int param) {
    prg = file_id = param;
    rows = prg.steps();
  }
  void draw() {
    // first line
    printfAt_P(0, 0, "%03d %08s %03d", 
      file_id, "", prg.duration());
    
    // second line
    printfAt_P(0, 1, "%02d/%02d   %c %03d %02d", 
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
        type_mode = prg.getMethod(row);
        type_duration = prg.getDuration(row);
        type_temp = prg.getTemperature(row); 
        break;
      case 'B':
        field = ( field - 1 + 3 ) % 3;
        break;
      case 'C':
        field = ( field + 1 + 3 ) % 3;
        break;
      case 'D':
        row = ( row + 1 + rows+1 ) % ( rows+1 );
        type_mode = prg.getMethod(row);
        type_duration = prg.getDuration(row);
        type_temp = prg.getTemperature(row); 
        break;
      case '0': case '1': case '2': case '3': case '4': 
      case '5': case '6': case '7': case '8': case '9': 
        switch (field) {
          case 0: 
            type_mode.on_key(key); 
            prg.setMethod(row, type_mode()); 
            break;
          case 1: 
            type_duration.on_key(key); 
            prg.setDuration(row, type_duration()); 
            break;
          case 2: 
            type_temp.on_key(key); 
            prg.setTemperature(row, type_temp()); 
            break;
        }
        break;
      case '*': next<program_save>();
    }
  }
  void on_back(int retVal) {
    if (retVal) {
      prg.saveChanges();
    }
    back();
  }
};

class program_save : public ux {
  void draw() {
    printScreen_P(
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
    printScreen_P(
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
      case 0: 
        if (check) {
          printAt_P(0, 0, "Check         OK"); 
        } else {
          printAt_P(0, 0, "Check      ERROR"); 
        }        
        break;
      case 1: printfAt_P(0, 0, "Used      %6d", used); break;
      case 2: printfAt_P(0, 0, "Free      %6d", free); break;
      case 3: printfAt_P(0, 0, "Total     %6d", total); break;
      case 4: printfAt_P(0, 0, "Files     %6d", files); break;
      case 5: printfAt_P(0, 0, "Max chunk %6d", max_free_chunk); break;
    }
    if (row == 0) {
      printAt_P(0, 1, "*-Back  Format-#");
    } else {
      printAt_P(0, 1, "*-Back          ");
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


