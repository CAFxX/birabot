volatile byte temperature_target = 0;

static void set_temperature_target(byte target) {
  temperature_target = target;
}

static byte get_temperature_target() {
  return temperature_target;
}

static void check_temperature() {
  flame(get_temperature_target() > get_temperature());
}
