#include "internal.hpp"
#include <dolphin/os.h>

#ifndef __GEKKO__

// On PC there are no hardware interrupts to manage. We track a fake interrupt-
// enabled state so that the common disable/save/restore pattern used by game
// code as a critical-section guard works correctly when nested.
static BOOL sInterruptsEnabled = TRUE;

void __RAS_OSDisableInterrupts_begin(void) {}
void __RAS_OSDisableInterrupts_end(void) {}

BOOL OSDisableInterrupts(void) {
  BOOL old = sInterruptsEnabled;
  sInterruptsEnabled = FALSE;
  return old;
}

BOOL OSEnableInterrupts(void) {
  BOOL old = sInterruptsEnabled;
  sInterruptsEnabled = TRUE;
  return old;
}

BOOL OSRestoreInterrupts(BOOL level) {
  BOOL old = sInterruptsEnabled;
  sInterruptsEnabled = level;
  return old;
}

#endif // !__GEKKO__
