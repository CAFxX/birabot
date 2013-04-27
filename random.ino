#include <limits.h>

void setup_random() {
  long seed = 0;
  for (int i=0; i<32; i++) {
    seed ^= analogRead(PIN_UNCONNECTED);
    randomSeed(seed);
    seed = random(LONG_MAX);
  }
}
