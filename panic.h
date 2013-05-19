#ifndef PANIC
#define PANIC

#include "display_utils.h"

#define __OK_PANIC__(msg, halt) { \
  handle_panic(); \
  clear_display(); \
  printAt_P(0, 0, msg); \
  printAt_P(0, 1, __FILE__); \
  printfAt_P(11, 1, " %4d", __LINE__); \
  while (halt); \
  reset(); \
}

#define __HALT__(msg) __OK_PANIC__(msg, true)
#define __PANIC__(msg) __OK_PANIC__(msg, false)

#endif // PANIC
