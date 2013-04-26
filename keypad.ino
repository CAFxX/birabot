#include "pins.h"

const byte rows = 4; // four rows
const byte cols = 4; // four columns
const char keys[rows][cols] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'#','0','*','D'}
};
byte rowPins[rows] = { PIN_KEYPAD_ROW0, PIN_KEYPAD_ROW1, PIN_KEYPAD_ROW2, PIN_KEYPAD_ROW3 }; // connect to the row pinouts of the keypad
byte colPins[rows] = { PIN_KEYPAD_COL0, PIN_KEYPAD_COL1, PIN_KEYPAD_COL2, PIN_KEYPAD_COL3 }; // connect to the row pinouts of the keypad
Keypad keypad( makeKeymap(keys), rowPins, colPins, rows, cols );

void keypadEvent(char key) {
}

void setup_keypad() {
  keypad.addEventListener(keypadEvent);  // Add an event listener.
  keypad.setHoldTime(500);               // Default is 1000mS
  keypad.setDebounceTime(250);           // Default is 50mS
}
