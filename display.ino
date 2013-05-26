#include "pins.h"
#include "custom_chars.h"
#include "Program.h"
#include "panic.h"
#include "display_utils.h"
#include "utils.h"

LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_ENABLE, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);

class main_menu;
class program_abort;
class program_run;
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
      case 'B': next<program_run>(); break;
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
      "*-Cont   Abort-#"
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

class program_run : public ux {
  byte file_id;
  bool go_back;
  public:
  program_run() : go_back(false), file_id(0) {
  }
  void draw() {
    // NOOP
  }
  void on_show() {
    if (!go_back) {
      go_back = true;
      next<program_list>();
    } else if (file_id != 0) {
      next<program_progress>(file_id);
    } else {
      back();
    }
  }
  void on_back(int retVal) {
    file_id = retVal;
  }
};

class program_list : public ux {
  bool typing;
  Program *prg;
  public:
  program_list() : prg(NULL), typing(false) {
    load_program(1);
  }
  ~program_list() {
    delete prg;
  }
  void draw() {
    char *desc;
    if (prg->is_valid()) {
      printfAt_P(0, 0, "%03d Program  %03d", prg->id(), prg->duration());
    } else if (fs.open(prg->id()).is_valid()) {
      if (prg->id() != 0) {
        printfAt_P(0, 0, "%03d Unknown file", prg->id());
      } else {
        printfAt_P(0, 0, "%03d Reserved    ", prg->id());
      }
    } else {
      printfAt_P(0, 0, "%03d Free        ", prg->id());
    }
    printAt_P(0, 1, "*-Back  Select-#");
  }
  void on_key(char key) {
    byte file_id = prg == NULL ? 1 : prg->id();
    switch (key) {
      case 'A': typing = false; load_program(prg->id() -  1); break;
      case 'B': typing = false; load_program(prg->id() - 10); break;
      case 'C': typing = false; load_program(prg->id() + 10); break;
      case 'D': typing = false; load_program(prg->id() +  1); break;
      case '0': case '1': case '2': case '3': case '4': 
      case '5': case '6': case '7': case '8': case '9': {
        ux_input_numeric<256, 100> new_file_id = typing ? prg->id() : 0;
        typing = true;
        new_file_id.on_key(key);
        load_program(new_file_id);
        break;
      }
      case '#': back(prg->id()); break;
      case '*': back(); break;
    }
  }
  void load_program(byte file_id) {
    delete prg;
    prg = new Program(file_id);
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
      case 'B': 
        next<program_setup>(retVal); 
        break;
      case 'C': 
        last_key = 'c'; 
        copy_source_file_id = retVal;
        next<program_list>(); 
        break;
      case 'c': {
        Serial.println(F("copying file"));
        microfsfile src = fs.open(copy_source_file_id);
        if (!src.is_valid()) {
          Serial.println(F("src not valid"));
        }
        microfsfile dst = fs.create(src.get_size(), retVal);
        if (!dst.is_valid()) {
          Serial.println(F("dst not valid"));
        }
        for (int i=0; i<src.get_size(); i++) {
          dst.write_byte(i, src.read_byte(i));
        }
        break;
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
  int s;
  Program *prg;
  time_t start;
  public:
  program_progress() : s(0), prg(NULL), start(now()) {
  }
  ~program_progress() {
    delete prg;
  }
  void on_show() {
    lcdCreateCharPGM(0, sym_ignition_off, false);  
    lcdCreateCharPGM(1, sym_ignition_off, true);  
    lcdCreateCharPGM(2, sym_gasvalve_off, false);  
    lcdCreateCharPGM(3, sym_gasvalve_off, true);  
    lcdCreateCharPGM(4, sym_flame_off, false);  
    lcdCreateCharPGM(5, sym_flame_off, true);  
    lcdCreateCharPGM(6, sym_alarm_off, false);  
    lcdCreateCharPGM(7, sym_alarm_off, true);  
  }
  void on_init(int param) {
    prg = new Program(param);
  }
  void draw() {
    int s = now() - start;
    
    set_temperature_target(prg->getTemperatureAt(s/60));

    // first line
    printfAt_P(0, 0, "%02d\xdf\x7e%02d\xdf  ", 
      get_temperature(), get_temperature_target());
    writeAt( 9, 0, false ? 7 : 6);
    writeAt(10, 0, ' '); 
    writeAt(11, 0, ignition_on() ? 1 : 0);
    writeAt(12, 0, ' '); 
    writeAt(13, 0, gasvalve_on() ? 3 : 2); 
    writeAt(14, 0, ' '); 
    writeAt(15, 0, flame_on() ? 5 : 4); 
    
    // second line
    printfAt_P(0, 1, "%03d      %03d+%03d", 
      prg->id(), s/60, prg->duration()-s/60);
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
    temp_valid = true;
  }

  void on_show() {
    lcdCreateCharPGM(0, sym_ignition_off, false);  
    lcdCreateCharPGM(1, sym_ignition_off, true);  
    lcdCreateCharPGM(2, sym_gasvalve_off, false);  
    lcdCreateCharPGM(3, sym_gasvalve_off, true);  
    lcdCreateCharPGM(4, sym_flame_off, false);  
    lcdCreateCharPGM(5, sym_flame_off, true);  
    lcdCreateCharPGM(6, sym_alarm_off, false);  
    lcdCreateCharPGM(7, sym_alarm_off, true);  
  }
  
  void draw() {
    // first line
    printfAt_P(0, 0, "%02d\xdf\x7e%02d\xdf  ", 
      get_temperature(), temp_set());
    writeAt( 9, 0, false ? 7 : 6);
    writeAt(10, 0, ' '); 
    writeAt(11, 0, !ignition_on() ? 1 : 0);
    writeAt(12, 0, ' '); 
    writeAt(13, 0, !gasvalve_on() ? 3 : 2); 
    writeAt(14, 0, ' '); 
    writeAt(15, 0, !flame_on() ? 5 : 4); 
    
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
  wrapping<byte, 3> field;
  byte row;
  byte rows;
  Program *prg;
  
  ux_input_numeric<2, 1> type_mode;
  ux_input_numeric<256, 100> type_duration;
  ux_input_numeric<100, 100> type_temp;
  
  public:
  program_setup() {
    row = 0;
    rows = 0;
    field = 0;
    prg = NULL;
  }
  ~program_setup() {
    delete prg;
  }
  void on_show() {
    lcdCreateCharPGM(1, type_constant, false);  
    lcdCreateCharPGM(2, type_linear, false);  
  }
  void on_init(int param) {
    Serial.println(F("on_init"));
    Serial.println(param);
    prg = new Program(param);
    reload();
  }
  void draw() {
    // first line
    printfAt_P(0, 0, "%03d          %03d", 
      prg->id(), prg->duration());
    
    // second line
    printfAt_P(0, 1, "%02d/%02d  %c%c%c%03d%c%02d", 
      row, rows, 
      field == 0 ? '\x7e' : ' ', type_mode() ? '\x01' : '\x02', 
      field == 1 ? '\x7e' : ' ', type_duration(), 
      field == 2 ? '\x7e' : ' ', type_temp());
  }
  void on_key(char key) {
    switch (key) {
      case 'A':
        row = ( row - 1 + rows ) % ( rows );
        reload();
        break;
      case 'B':
        field--;
        break;
      case 'C':
        field++;
        break;
      case 'D':
        row = ( row + 1 + rows ) % ( rows );
        reload();
        break;
      case '0': case '1': case '2': case '3': case '4': 
      case '5': case '6': case '7': case '8': case '9': 
        if (rows == 0) {
          prg->addStep();
          reload();
        }
        switch (field) {
          case 0: 
            type_mode.on_key(key); 
            prg->setMethod(row, type_mode()); 
            break;
          case 1: 
            type_duration.on_key(key); 
            prg->setDuration(row, type_duration()); 
            break;
          case 2: 
            type_temp.on_key(key); 
            prg->setTemperature(row, type_temp()); 
            break;
        }
        break;
      case '*': 
        next<program_save>(); 
        break;
      case '#': 
        prg->addStep(++row);
        reload();
        break;
    }
  }
  void on_back(int retVal) {
    if (retVal) {
      prg->saveChanges();
    }
    back();
  }
  void reload() {
    rows = prg->steps();
    if (rows != 0) {
      type_mode = prg->getMethod(row);
      type_duration = prg->getDuration(row);
      type_temp = prg->getTemperature(row); 
    } else {
      type_mode = 0;
      type_duration = 0;
      type_temp = 0;
    }
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
  wrapping<int, 8> row;
  boolean check;
  size_t used;
  size_t free;
  size_t total;
  byte files;
  byte max_free_chunk;
  byte free_chunks;
  public:
  microfs_tool() {
    row = 0;
    check = fs.check_disk();
    used = fs.used();
    free = fs.free();
    total = fs.total();
    files = fs.files();
    max_free_chunk = fs.max_free_chunk();
    free_chunks = fs.free_chunks();
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
      case 1: printfAt_P(0, 0, "Used      %6d",    used); break;
      case 2: printfAt_P(0, 0, "Free      %6d",    free); break;
      case 3: printfAt_P(0, 0, "Total     %6d",    total); break;
      case 4: printfAt_P(0, 0, "Files        %3d", files); break;
      case 5: printfAt_P(0, 0, "Max filesize %3d", max_free_chunk); break;
      case 6: printfAt_P(0, 0, "Free chunks  %3d", free_chunks); break;
      case 7: printAt_P(0, 0,  "Dump            "); break;
    }
    switch (row) {
      default: printAt_P(0, 1, "*-Back          "); break;
      case 0:  printAt_P(0, 1, "*-Back  Format-#"); break;
      case 7:  printAt_P(0, 1, "*-Back    Dump-#"); break;
    }
  }
  void on_key(char key) {
    switch (key) {
      case 'A': row--; break;
      case 'D': row++; break;
      case '#': 
        switch (row) {
          case 0:  next<reset_confirm>(); break;
          case 7:  fs.dump(); break;
        } 
        break;
      case '*': back(); break;
    }
  }
};


