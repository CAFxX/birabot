#include <OneWire.h>

OneWire temp_sensor(PIN_TEMP_SENS);

byte temp_sensor_addr[8] = {0};
boolean temp_sensor_found = false;
int8_t temp_sensor_value = 0;
uint8_t temp_sensor_value_age = 0;

static void setup_temperature() {
  // search for the sensor
  temp_sensor_found = ds.search(temp_sensor_addr);
  if (!temp_sensor_found) {
    ds.reset_search();
  }
  if (!check_CRC(temp_sensor_addr, 8)) {
    temp_sensor_found = false;
    ds.reset_search();
    return;
  }
  // configure the sensor
  ds.reset();
  ds.select(temp_sensor_addr);
  ds.write(0x4E);
  ds.write(0x00); // Th
  ds.write(0x00); // Tl
  ds.write(0x00); // configuration
  ds.reset();
}

static boolean check_CRC(byte* data, int len) {
  return OneWire::CRC(data, len-1) == data[len-1];
}

static void refresh_temperature() {
  // request a new sample
  ds.reset();
  ds.select(temp_sensor_addr);
  ds.write(0x44, 1);
  // wait for it
  delay(100); // ms
  // fetch the sample
  if (!ds.reset())
    return;
  ds.select(temp_sensor_addr);    
  ds.write(0xBE);
  byte buf[8];
  for (byte i=0; i<8; i++) {
    buf[8] = ds.read();
  }
  ds.reset();
  // check that the data has been trasnferred correctly
  if (!check_CRC(buf, 8))
    return;
  // all went well: update the global values
  int16_t celsius_16 = b2i16(buf[0], buf[1]);
  temp_sensor_value = celsius_16 >> 4;
  temp_sensor_value_age = 0;
}

static int8_t get_temperature() {
  return temp_sensor_value;
}

