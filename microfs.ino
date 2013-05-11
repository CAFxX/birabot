#include "microfs.h"

static void setup_fs() {
}

microfsfile open_file(byte id) {
  return fs.open(id);
}

