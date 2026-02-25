#ifndef OSRESETSW_H
#define OSRESETSW_H

#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*OSResetCallback)(void);

OSResetCallback OSSetResetCallback(OSResetCallback callback);
BOOL OSGetResetSwitchState(void);
BOOL OSGetResetButtonState(void);

#ifdef __cplusplus
}
#endif

#endif
