#include "microfs.h"

static void setup_fs() {
  setup_panic(1, !fs.check_disk());
}

