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

PROGMEM const char alphakeys[] = {
  /*  0 */ ' ', '-', '0',
  /*  3 */ 'A', 'B', '1',
  /*  6 */ 'C', 'D', '2',
  /*  9 */ 'E', 'F', '3',
  /* 12 */ 'G', 'H', '4',
  /* 15 */ 'I', 'J', 'K', '5',
  /* 19 */ 'L', 'M', 'N', '6',
  /* 23 */ 'O', 'P', 'Q', '7',
  /* 27 */ 'R', 'S', 'T', '8',
  /* 31 */ 'U', 'V', 'W', 'X', 'Y', 'Z', '9' /* 38 */
};

PROGMEM const byte alphakeys_idx[] = {
  0, 3, 6, 9, 12, 15, 19, 23, 27, 31, 38
};

Keypad keypad( makeKeymap(keys), rowPins, colPins, rows, cols );

void keypadEvent(char key) {
  if (keypad.getState() == PRESSED) {
    uxmgr::get().on_key(key);
  }
}

static void setup_keypad() {
  keypad.addEventListener(keypadEvent);  
}
