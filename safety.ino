// flame_level must be greater than flame_level_threshold to be considered OK
byte flame_level_threshold = 255;
// the ignition phase must end before this threshold (multiplied by safety_control_interval)
byte safety_ignition_override_threshold = 100; 
// safety_control will be run each safety_control_interval ms
int safety_control_interval = 50; // ms

volatile boolean safety_gasvalve_on = false;
volatile byte flame_level = 0;
volatile byte safety_ignition_override = 255;

static void safety_control() {
  safety_gasvalve_on = false;
  if (ignition_on()) {
    // we are in the ignition phase
    safety_gasvalve_on = true;
    safety_ignition_override++;
  } else {
    // normal phase
    if (flame_on()) {
      safety_gasvalve_on = true;
    }
  }
}

boolean safety_control_ignition_override() {
  if (ignition_on())
    return false;
  safety_ignition_override = 0;
  return true;
}

boolean gasvalve_on() {
  return safety_gasvalve_on;
}

boolean ignition_on() {
  return safety_ignition_override < safety_ignition_override_threshold;
}

boolean flame_on() {
  return flame_level > flame_level_threshold;
}

void setup_safety() {
  // the safety_control routine is called every 50ms
  FlexiTimer2::set(safety_control_interval, safety_control);
  FlexiTimer2::start();
}

