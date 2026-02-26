#include <dolphin/ar.h>
#include "ar.hpp"

#define ARQ_CHUNK_SIZE 0x1000

static u32 __ARQChunkSize;
static BOOL __ARQ_init_flag;

void __ARQCallbackHack([[maybe_unused]] uintptr_t request_addr) {}

void ARQInit(void) {
  if (__ARQ_init_flag != TRUE) {
    __ARQChunkSize = ARQ_CHUNK_SIZE;
    __ARQ_init_flag = TRUE;
  }
}

void ARQReset(void) { __ARQ_init_flag = FALSE; }

void ARQPostRequest(ARQRequest* request, u32 owner, u32 type, u32 priority, u32 source, u32 dest, u32 length,
                    ARQCallback callback) {
  if (request == nullptr) {
    Log.fatal("'request' is null");
  }

  if (type != ARQ_TYPE_MRAM_TO_ARAM && type != ARQ_TYPE_ARAM_TO_MRAM) {
    Log.fatal("Bad 'type' parameter {}", type);
  }

  if (priority != ARQ_PRIORITY_LOW && priority != ARQ_PRIORITY_HIGH) {
    Log.warn("Bad 'priority' parameter {}", priority);
  }

  if ((length % ARQ_DMA_ALIGNMENT) != 0) {
    Log.warn("Unaligned DMA length {:X}", length);
  }

  if ((source % ARQ_DMA_ALIGNMENT) != 0) {
    Log.warn("Unaligned DMA source {:X}", source);
  }

  if ((dest % ARQ_DMA_ALIGNMENT) != 0) {
    Log.warn("Unaligned DMA dest {:X}", dest);
  }

  // We don't care about the request,
  // but the caller may use data set
  request->next = NULL;
  request->owner = owner;
  request->type = type;
  request->source = source;
  request->dest = dest;
  request->length = length;
  request->callback = callback != nullptr ? callback : __ARQCallbackHack;

  // NOTE: we do not set request->priority,
  // the SDK does not set it either.

  // Forward to AR directly
  u32 mainramAddr;
  u32 aramAddr;

  if (type == ARQ_TYPE_MRAM_TO_ARAM) {
    mainramAddr = source;
    aramAddr = dest;
  } else {
    mainramAddr = dest;
    aramAddr = source;
  }

  ARStartDMA(type, mainramAddr, aramAddr, length);
  request->callback((uintptr_t)request);
}

void ARQRemoveRequest([[maybe_unused]] ARQRequest* request) {
  // no-op
}

void ARQRemoveOwnerRequest([[maybe_unused]] u32 owner) {
  // no-op
}

void ARQFlushQueue(void) {
  // no-op
}

void ARQSetChunkSize(u32 size) { __ARQChunkSize = ALIGN(size, ARQ_DMA_ALIGNMENT); }

u32 ARQGetChunkSize(void) { return __ARQChunkSize; }

BOOL ARQCheckInit(void) { return __ARQ_init_flag; }
