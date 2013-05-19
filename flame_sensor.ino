

static void setup_flame_sensor() {
}

static void poll_flame_sensor() {
  int value = analogRead(PIN_FLAME_SENS);
  set_flame_level(value/4); // FIXME
}


