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
  lcd.begin(20, 4);
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
    loadLogo();
  }
  void draw() {
    printAt_P(7, 1, "BIRABOT");
    printAt_P(7, 2, __DATE__);
    // draw the logo
    drawLogo(2, 1);
  }
  void on_key(char) {
    show<main_menu>();
  };
};

class main_menu : public ux {
  void draw() {
    printScreen_P4(
      "       BIRABOT      ",
      "A-Manual Automatic-B",
      "C-Recipes    Tools-D",
      "                    "
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
    printScreen_P2(
      "   ABORT PROGRAM?   ",
      "*-Continue   Abort-#"
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
    set_temperature_target(0);
    fs.remove(1);
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
    load_program(2);
  }
  ~program_list() {
    delete prg;
  }
  void draw() {
    char *desc;
    printAt_P(0, 3, "    PROGRAM LIST    ");
    if (prg->is_valid()) {
      printfAt_P(0, 1, "%03d Program      %03d", prg->id(), prg->duration());
    } else if (fs.open(prg->id()).is_valid()) {
      if (prg->id() == 0 || prg->id() == 1) {
        printfAt_P(0, 1, "%03d Reserved        ", prg->id());
      } else {
        printfAt_P(0, 1, "%03d Unknown file    ", prg->id());
      }
    } else {
      printfAt_P(0, 1, "%03d Free            ", prg->id());
    }
    printAt_P(0, 3, "*-Back      Select-#");
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
    printScreen_P4(
      "   RECIPE EDITOR    ",
      "A-Create      Edit-B",
      "C-Copy      Delete-D",
      "*-Back              "
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
   +--------------------+
   |    RECIPE MODE     |
   |00°>00°      a i g f|
   |000          000+000|
   |*-Abort             |
   +--------------------+ 
   Progress screen shown during program execution
*/
class program_progress : public ux {
  int s;
  Program *prg;
  time_t start_t;
  time_t resume_t;
  public:
  program_progress() : s(0), prg(NULL), start_t(now()), resume_t(0) {
  }
  ~program_progress() {
    delete prg;
  }
  void on_show() {
    loadSymbols();
  }
  void on_init(int param) {
    prg = new Program(param);
    byte __resume_file_id = resume_file_id();
    if (__resume_file_id == prg->id()) {
      int seconds = resume_seconds();
      start_t -= seconds;
    }
  }
  void draw() {
    time_t second = (now() - start_t), minute = second / 60;
    if (now() - resume_t > 15) {
      resume_save(prg->id(), second);
      resume_t = now();
    }
    
    set_temperature_target(prg->getTemperatureAt(minute));

    printAt_P(0, 0, "    RECIPE MODE     ");

    printfAt_P(0, 1, "%02d\xdf\x7e%02d\xdf      ", 
      get_temperature(), get_temperature_target());
    writeAt(13, 1, alarm_on() ? 7 : 6);
    writeAt(14, 1, ' '); 
    writeAt(15, 1, ignition_on() ? 1 : 0);
    writeAt(16, 1, ' '); 
    writeAt(17, 1, gasvalve_on() ? 3 : 2); 
    writeAt(18, 1, ' '); 
    writeAt(19, 1, flame_on() ? 5 : 4); 
    
    printfAt_P(0, 2, "%03u          %03u+%03u", 
      prg->id(), (unsigned)minute, (unsigned)(prg->duration()-minute));
    
    printAt_P(0, 3, "*-Abort             ");
  }
  
  void on_key(char key) {
    switch (key) {
      case '*': 
        next<program_abort>(); 
        break;
    }
  }
  
};

/* 
   +--------------------+
   |    MANUAL MODE     |
   |00°>00°      a i g f|
   |0-9-Temperature     |
   |*-Stop              |
   +--------------------+ 
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
    set_temperature_target(temp_set());
  }

  void on_show() {
    loadSymbols();
  }
  
  void draw() {
    printAt_P(0, 0, "    MANUAL MODE     ");

    printfAt_P(0, 1, "%02d\xdf\x7e%02d\xdf      ", 
      get_temperature(), temp_set());
    writeAt(13, 1, alarm_on() ? 7 : 6);
    writeAt(14, 1, ' '); 
    writeAt(15, 1, ignition_on() ? 1 : 0);
    writeAt(16, 1, ' '); 
    writeAt(17, 1, gasvalve_on() ? 3 : 2); 
    writeAt(18, 1, ' '); 
    writeAt(19, 1, flame_on() ? 5 : 4); 
    
    printAt_P(0, 2, "0-9-Temperature     ");
    if (temp_valid) {
      printAt_P(0, 3, "*-Stop              ");
    } else {
      printAt_P(0, 3, "*-Cancel       Set-#");
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
        set_temperature_target(temp_set());
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
    printAt_P(0, 0, "    PROGRAM EDITOR    ");
    
    printfAt_P(0, 1, "%03d              %03d", 
      prg->id(), prg->duration());
    
    printfAt_P(0, 2, "%02d/%02d      %c%c%c%03d%c%02d", 
      row, rows, 
      field == 0 ? '\x7e' : ' ', type_mode() ? '\x01' : '\x02', 
      field == 1 ? '\x7e' : ' ', type_duration(), 
      field == 2 ? '\x7e' : ' ', type_temp());

    printAt_P(0, 3, "*-Back ^ADv \x7f" "BC\x7e Ins-#");
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
    printScreen_P2(
      "    SAVE CHANGES?   ",
      "*-Discard     Save-#"
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
    printScreen_P2(
      "  DELETE ALL DATA?  ",
      "*-Abort     Delete-#"
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
    printAt_P(0, 0, "       TOOLS        ");
    clearLine(1);
    clearLine(2);
    switch (row) {          
      case 0: printAt_P(0, 1, "Filesystem status");  if (check) { printAt_P(17, 2, "OK") } else { printAt_P(14, 2, "ERROR"); } break;
      case 1: printAt_P(0, 1, "Used space");         printfAt_P(13, 2, "%6d", used); break;
      case 2: printAt_P(0, 1, "Free space");         printfAt_P(13, 2, "%6d", free); break;
      case 3: printAt_P(0, 1, "Total space");        printfAt_P(13, 2, "%6d", total); break;
      case 4: printAt_P(0, 1, "Files");              printfAt_P(13, 2, "%6d", files); break;
      case 5: printAt_P(0, 1, "Largest free chunk"); printfAt_P(13, 2, "%6d", max_free_chunk); break;
      case 6: printAt_P(0, 1, "Free chunks");        printfAt_P(13, 2, "%6d", free_chunks); break;
      case 7: printAt_P(0, 1, "Export contents"); break;
    }
    switch (row) {
      default: printAt_P(0, 3, "*-Back              "); break;
      case 0:  printAt_P(0, 3, "*-Back      Format-#"); break;
      case 7:  printAt_P(0, 3, "*-Back        Dump-#"); break;
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


