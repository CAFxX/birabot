#include <OneWire.h>

OneWire temp_sensor(PIN_TEMP_SENS);

byte temp_sensor_addr[8];
boolean temp_sensor_found = false;
int8_t temp_sensor_value = 0;
uint8_t temp_sensor_value_age = 0;

static void setup_temperature() {
  // search for the sensor
  temp_sensor_found = temp_sensor.search(temp_sensor_addr);
  if (!temp_sensor_found) {
    // no onewire device found
    __HALT__("T sensor not found");
  }
  if (!check_CRC(temp_sensor_addr, 8)) {
    // crc validation error
    __HALT__("T sensor CRC err");
  }
  if (temp_sensor_addr[0] != 0x20) {
    // not a temperature sensor
    __HALT__("T sensor unknown");
  }
  // configure the sensor
  temp_sensor.reset();
  temp_sensor.select(temp_sensor_addr);
  temp_sensor.write(0x4E);
  temp_sensor.write(0x00); // Th
  temp_sensor.write(0x00); // Tl
  temp_sensor.write(0x00); // configuration
  temp_sensor.reset();
}

// asynchronously fetch the temperature
static void poll_temperature() {
  static boolean reqPending = false;
  static int reqTime = 0;
  int curTime = millis();
  const int sampleTime = 100; // ms
  if ( curTime-reqTime > sampleTime ) {
    if ( reqPending ) {
      read_sample();
      reqPending = false;
    }
    request_sample();
    reqTime = curTime;
    reqPending = true;
  }
}

// given a high byte and low byte, build a int16_t out of them
static int16_t b2i16(byte low, byte high) {
  int16_t val = (int8_t)high;
  return (val << 8) | low;
}

// check that the contents of the buffer are valid according to 
// the embedded CRC8
static boolean check_CRC(byte* data, int len) {
  return OneWire::crc8(data, len-1) == data[len-1];
}

static void request_sample() {
  temp_sensor.reset();
  temp_sensor.select(temp_sensor_addr);
  temp_sensor.write(0x44, 1);
}

static void read_sample() {
  // fetch the sample
  if (!temp_sensor.reset())
    return;
  temp_sensor.select(temp_sensor_addr);    
  temp_sensor.write(0xBE);
  byte buf[9];
  for (byte i=0; i<sizeof(buf); i++) {
    buf[i] = temp_sensor.read();
  }
  temp_sensor.reset();
  // check that the data has been trasnferred correctly
  if (!check_CRC(buf, sizeof(buf)))
    return;
  // all went well: update the global values
  int16_t celsius_16 = b2i16(buf[0], buf[1]);
  temp_sensor_value = celsius_16 >> 4;
  temp_sensor_value_age = 0;
}

static int8_t get_temperature() {
  return temp_sensor_value;
}

