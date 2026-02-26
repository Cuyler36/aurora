#include <dolphin/ar.h>
#include <dolphin/os.h>
#include "ar.hpp"
#include <cstring>

#define AR_CHK_ADDR(addr, len, min, max)                                                                               \
  (((uintptr_t)(addr)) >= ((uintptr_t)(min)) && ((uintptr_t)(addr) + (len) <= ((uintptr_t)(max))))

static ARCallback __AR_Callback;
static u32 __AR_Size;
static u32 __AR_InternalSize;
static u32 __AR_ExpansionSize;
static u32 __AR_StackPointer;
static u32 __AR_FreeBlocks;
static u32* __AR_BlockLength;
static BOOL __AR_init_flag;

// aurora
static void* ARAMStart;
static void* ARAMEnd;

ARCallback ARRegisterDMACallback(ARCallback callback) {
  ARCallback old_callback = __AR_Callback;
  BOOL old = OSDisableInterrupts();
  __AR_Callback = callback;
  OSRestoreInterrupts(old);
  return old_callback;
}

u32 ARGetDMAStatus(void) {
  return 0; // We are never in a DMA state
}

void ARStartDMA(u32 type, u32 mainmem_addr, u32 aram_addr, u32 length) {
  void* src;
  void* dst;

  // We will not enforce these, but the user should be warned
  if (!AR_ALIGN_CHK(mainmem_addr)) {
    Log.warn("Misaligned mainmem_addr {:X}", mainmem_addr);
  }
  if (!AR_ALIGN_CHK(aram_addr)) {
    Log.warn("Misaligned aram_addr {:X}", aram_addr);
  }
  if (!AR_ALIGN_CHK(length)) {
    Log.warn("Misaligned ARAM transfer length {:X}", length);
  }

  if (type == ARAM_DIR_MRAM_TO_ARAM) {
    src = (void*)mainmem_addr;
    dst = (void*)((uintptr_t)ARAMStart + aram_addr);
  } else {
    src = (void*)((uintptr_t)ARAMStart + aram_addr);
    dst = (void*)mainmem_addr;
  }

  if (!AR_CHK_ADDR(mainmem_addr, length, OSBaseAddress, OSBaseAddress + OSGetPhysicalMemSize())) {
    Log.fatal("Attempt to access beyond configured MEM1! {:08X}-{:08X}", (uintptr_t)mainmem_addr,
              (uintptr_t)mainmem_addr + length);
  }

  if (!AR_CHK_ADDR(aram_addr, length, 0, __AR_Size)) {
    Log.fatal("Attempt to access beyond configured ARAM! {:08X}-{:08X}", (uintptr_t)aram_addr,
              (uintptr_t)aram_addr + length);
  }

  memcpy(dst, src, length);

  if (__AR_Callback) {
    __AR_Callback();
  }
}

u32 ARAlloc(u32 length) {
  u32 tmp;
  BOOL old = OSDisableInterrupts();

  if (!AR_ALIGN_CHK(length)) {
    // Since we're allocating entire blocks we have to round
    Log.warn("Misaligned alloc length {:X}, rounding up", length);
    length = ALIGN(length, ARQ_DMA_ALIGNMENT);
  }

  tmp = __AR_StackPointer;
  __AR_StackPointer += length;
  *__AR_BlockLength = length;
  __AR_BlockLength++;
  __AR_FreeBlocks--;
  OSRestoreInterrupts(old);
  return tmp;
}

u32 ARFree(u32* length) {
  BOOL old = OSDisableInterrupts();

  __AR_BlockLength--;
  if (length) {
    *length = *__AR_BlockLength;
  }

  __AR_StackPointer -= *__AR_BlockLength;
  __AR_FreeBlocks++;
  OSRestoreInterrupts(old);
  return __AR_StackPointer;
}

BOOL ARCheckInit(void) { return __AR_init_flag; }

u32 ARInit(u32* stack_index_addr, u32 num_entries) {
  BOOL old;

  if (__AR_init_flag == TRUE) {
    return AR_USER_BASE;
  }

  old = OSDisableInterrupts();
  __AR_Callback = NULL;
  __AR_StackPointer = AR_USER_BASE;
  __AR_FreeBlocks = num_entries;
  __AR_BlockLength = stack_index_addr;
  __AR_Size = aurora::g_config.aramSize;
  __AR_InternalSize = aurora::g_config.aramSize;
  __AR_init_flag = TRUE;

  // Setup internal ARAM buffer
  ARAMStart = calloc(__AR_Size, 1);

  if (ARAMStart == nullptr) {
    Log.fatal("Failed to allocate ARAM buffer");
  }

  ARAMEnd = (void*)((uintptr_t)ARAMStart + __AR_Size);

  OSRestoreInterrupts(old);
  return __AR_StackPointer;
}

void ARReset(void) {
  __AR_init_flag = FALSE;

  if (ARAMStart != nullptr) {
    free(ARAMStart);
    ARAMStart = nullptr;
  }

  ARAMEnd = nullptr;
}

void ARSetSize(void) { Log.warn("ARSetSize() is deprecated"); }

u32 ARGetBaseAddress(void) { return AR_USER_BASE; }

u32 ARGetSize(void) { return __AR_Size; }

u32 ARGetInternalSize(void) { return __AR_InternalSize; }

void ARClear(u32 flag) {
  u32 ofs = 0;
  u32 size = 0;

  switch (flag) {
  case AR_CLEAR_ALL:
    size = __AR_InternalSize;
    break;
  case AR_CLEAR_USER:
    ofs = AR_USER_BASE;
    size = __AR_InternalSize - AR_USER_BASE;
    break;
  case AR_CLEAR_EXPANSION:
    Log.warn("ARClear(): attempt to clear unused expansion area");
    ofs = __AR_InternalSize;
    size = __AR_ExpansionSize; // should always be 0 in our case
    break;
  }

  if (size != 0) {
    memset((void*)((uintptr_t)ARAMStart + ofs), 0, size);
  }
}
