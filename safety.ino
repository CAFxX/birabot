#include <avr/io.h>
#include <avr/wdt.h>

// flame_level must be greater than flame_level_threshold to be considered OK
byte flame_level_threshold = 255;
// the ignition phase must end before this threshold (multiplied by safety_control_interval)
byte safety_ignition_override_threshold = 60; // 60*50ms = 3s
// safety_control will be run each safety_control_interval ms
int safety_control_interval = 50; // ms

volatile boolean safety_gasvalve_on = false;
volatile byte flame_level = 0;
volatile byte safety_ignition_override = 255;
volatile boolean watchdog_expire = false;
volatile boolean flame_required = false;

static void safety_control() {
  // the valve safety is "normally closed"
  safety_gasvalve_on = false;
  // check if the flame needs to be on
  if (flame_required) {
    // check if we are in the ignition phase
    if (ignition_on()) {
      // we are in the ignition phase
      safety_gasvalve_on = true;
      safety_ignition_override++;
    } else if (flame_on()) {
      // normal phase
      safety_gasvalve_on = true;
      safety_ignition_override = 255;
    } else {
      // the flame is required but we are not in the ignition phase, or in the normal phase
      // i.e. either we could not ignite the flame, or the flame died for some other reason (wind?)
      // TODO: if we just tried igniting, wait for a bit and then retry (a couple of times); 
      // if the flame was on, retry immediately, then wait and retry (a couple of times)
    }
  }
  // set the ignition and gas valve output pins as decided above
  write_output_pins();
  // finally, prevent the watchdog (250ms) from resetting the arduino
  // unless the watchdog_expire flag has been set
  if (watchdog_expire == false) {
    wdt_reset();
  }
}

void write_output_pins() {
  digitalWrite(PIN_IGNITION, ignition_on());
  digitalWrite(PIN_GASVALVE, gasvalve_on());
}

boolean safety_control_ignition_override() {
  if (ignition_on())
    return false;
  safety_ignition_override = 0;
  return true;
}

void reset() {
  if (watchdog_expire) {
    return;
  }
  safety_gasvalve_on = false; // shut off the gas valve
  safety_ignition_override = 255; // stop ignition
  wdt_reset(); // wait other 250ms before resetting
  write_output_pins();
  watchdog_expire = true;
}

void flame(boolean on) {
  // if a reset was requested, do not allow controlling the flame
  if (watchdog_expire) {
    return;
  }
  if (on && !flame_required) {
    // start ignition
    safety_ignition_override = 0; 
  }
  flame_required = on;
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
  // the watchdog is set to 250ms and is reset only in the safety_control
  wdt_enable(WDTO_250MS);
}

