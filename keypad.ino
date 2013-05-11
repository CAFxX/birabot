#include "pins.h"

const byte rows = 4; 
const byte cols = 4; 

byte rowPins[rows] = { PIN_KEYPAD_ROW0, PIN_KEYPAD_ROW1, PIN_KEYPAD_ROW2, PIN_KEYPAD_ROW3 }; 
byte colPins[cols] = { PIN_KEYPAD_COL0, PIN_KEYPAD_COL1, PIN_KEYPAD_COL2, PIN_KEYPAD_COL3 }; 

const char keys[rows][cols] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'#','0','*','D'}
};

Keypad keypad( makeKeymap(keys), rowPins, colPins, rows, cols );

void keypadEvent(char key) {
  if (keypad.getState() == PRESSED) {
    uxmgr::on_key(key);
  }
}

static void setup_keypad() {
  keypad.addEventListener(keypadEvent);  
}
