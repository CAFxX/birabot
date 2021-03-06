// LCD pins
// note: we use 4 bit mode, so we need only 4 data lines (D4-D7)
#define PIN_LCD_RS 12
#define PIN_LCD_ENABLE 11
#define PIN_LCD_D4 5
#define PIN_LCD_D5 4
#define PIN_LCD_D6 3
#define PIN_LCD_D7 2

// Keypad pins
// note: the Keypad library will us pinMode INPUT_PULLUP on either row or col pins
// this means the LED pin should not be used as a keypad pin!
#define PIN_KEYPAD_ROW0 A2
#define PIN_KEYPAD_ROW1 A3
#define PIN_KEYPAD_ROW2 A4
#define PIN_KEYPAD_ROW3 A5
#define PIN_KEYPAD_COL0 6
#define PIN_KEYPAD_COL1 7
#define PIN_KEYPAD_COL2 8
#define PIN_KEYPAD_COL3 9

// Temperature sensor data pin (I2C)
#define PIN_TEMP_SENS A1

// Flame sensor analog input
#define PIN_FLAME_SENS A0

// Ignition output pin
#define PIN_IGNITION 10

// Gas valve output pin
#define PIN_GASVALVE 13

// Unconnected analog pin to be used as seed for the RNG
#define PIN_UNCONNECTED A1

// Alarm pin
#define PIN_ALARM A3

// Led pin
#define PIN_LED LED_BUILTIN
