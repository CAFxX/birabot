#include <avr/io.h>
#include <avr/wdt.h>

// flame_level must be greater than flame_level_threshold to be considered OK
byte flame_level_threshold = 1;
// the ignition phase must end before this threshold (multiplied by safety_control_interval)
byte safety_ignition_override_threshold = 60; // 60*50ms = 3s
// safety_control will be run each safety_control_interval ms
int safety_control_interval = 50; // ms
// distance between ignition attempts (multiplied by safety_control_interval)
int safety_ignition_distance_threshold = 300; // 300*50ms = 15s
// maximum number of attempts before panic
byte safety_ignition_attempts_threshold = 3;

volatile boolean safety_gasvalve_on = false;
volatile byte flame_level = 0;
volatile byte safety_ignition_override = 255;
volatile int safety_ignition_distance = 0;
volatile byte safety_ignition_attempts = 0;
volatile boolean watchdog_expire = false;
volatile boolean flame_required = false;
volatile boolean safety_flame_required = false;
volatile boolean safety_alarm = false;

static void safety_control() {
  // the valve safety is "normally closed"
  safety_gasvalve_on = false;
  // if a reset is pending everything must be off
  if (watchdog_expire == false) {
    if (flame_required && !safety_flame_required) {
      safety_control_ignition_override();
    }
    safety_flame_required = flame_required;
    // check if the flame needs to be on
    if (flame_required) {
      // check if we are in the ignition phase
      if (ignition_on()) {
        // we are in the ignition phase
        safety_gasvalve_on = true;
        safety_ignition_override++;
        safety_ignition_distance = 0;
      } else if (flame_on()) {
        // normal phase
        safety_gasvalve_on = true;
        safety_ignition_override = 255;
        safety_ignition_distance = 0;
      } else {
        // the flame is required but we are not in the ignition phase, or in the normal phase
        // i.e. either we could not ignite the flame, or the flame died for some other reason (wind?)
        // TODO: if we just tried igniting, wait for a bit and then retry (a couple of times); 
        // if the flame was on, retry immediately, then wait and retry (a couple of times)
        safety_ignition_distance++;
        if (safety_ignition_distance >= safety_ignition_distance_threshold && safety_ignition_attempts < safety_ignition_attempts_threshold) {
          safety_ignition_attempts++;
          safety_control_ignition_override();
        } else if (safety_ignition_attempts >= safety_ignition_attempts_threshold) {
          handle_panic();
          safety_alarm = true;
        } else {
          // just wait...
        }
      }
    } else {
      handle_panic();
    }
    // set the ignition and gas valve output pins as decided above
    write_output_pins();
    // finally, prevent the watchdog (250ms) from resetting the arduino
    // unless the watchdog_expire flag has been set
    wdt_reset();
  }
}

static void write_output_pins() {
  digitalWrite(PIN_IGNITION, ignition_on());
  digitalWrite(PIN_GASVALVE, gasvalve_on());
}

static boolean safety_control_ignition_override() {
  if (ignition_on() || alarm_on())
    return false;
  safety_ignition_override = 0;
  return true;
}

// this is an emergency procedure that shuts off all "dangerous" activities
// note that it won't prevent other code from restarting such activities!
static void handle_panic() {
  safety_gasvalve_on = false; // shut off the gas valve
  safety_ignition_override = 255; // stop ignition
  write_output_pins();
}

// this function resets the birabot
// note that control never returns from this function!
static void reset() {
  handle_panic();
  wdt_disable();
  wdt_enable(WDTO_15MS);
  watchdog_expire = true;
  while (true)
    ;
}

static void flame(boolean on) {
  flame_required = on;
}

static boolean gasvalve_on() {
  return safety_gasvalve_on;
}

static boolean ignition_on() {
  return safety_ignition_override < safety_ignition_override_threshold;
}

static boolean flame_on() {
  return flame_level > flame_level_threshold;
}

static boolean alarm_on() {
  return safety_alarm;
}

static void set_flame_level(byte level) {
  flame_level = level;
}

static byte get_flame_level() {
  return flame_level;
}

static void setup_safety() {
  wdt_disable();
  wdt_enable(WDTO_250MS);
  // the safety_control routine is called every 50ms
  FlexiTimer2::set(safety_control_interval, safety_control);
  FlexiTimer2::start();
}


