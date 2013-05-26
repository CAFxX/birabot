// LCD pins
// note: we use 4 bit mode, so we need only 4 data lines (D4-D7)
#define PIN_LCD_RS 7
#define PIN_LCD_ENABLE 6
#define PIN_LCD_D4 2
#define PIN_LCD_D5 3
#define PIN_LCD_D6 4
#define PIN_LCD_D7 5

// Keypad pins
// note: the Keypad library will us pinMode INPUT_PULLUP on either row or col pins
// this means the LED pin should not be used as a keypad pin!
#define PIN_KEYPAD_COL0 10
#define PIN_KEYPAD_COL1 11
#define PIN_KEYPAD_COL2 12
#define PIN_KEYPAD_COL3 13
#define PIN_KEYPAD_ROW0 A0
#define PIN_KEYPAD_ROW1 A1
#define PIN_KEYPAD_ROW2 A2
#define PIN_KEYPAD_ROW3 A3

// Temperature sensor data pin (I2C)
#define PIN_TEMP_SENS A0

// Flame sensor analog input
#define PIN_FLAME_SENS A1

// Ignition output pin
#define PIN_IGNITION A5

// Gas valve output pin
#define PIN_GASVALVE A6

// Unconnected analog pin to be used as seed for the RNG
#define PIN_UNCONNECTED A4

// Alarm pin
#define PIN_ALARM A3

// Led pin
#define PIN_LED LED_BUILTIN
