#ifndef OS_ARENA_H
#define OS_ARENA_H
#include <dolphin/types.h>

#ifdef __cplusplus
extern "C"{
#endif

void* OSGetArenaHi(void);
void* OSGetArenaLo(void);

void OSSetArenaHi(void*);
void OSSetArenaLo(void*);

#ifdef __cplusplus
}
#endif
#endif