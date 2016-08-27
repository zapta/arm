#ifndef SYSTEM_MEMORY_H
#define SYSTEM_MEMORY_H

#include "util/common.h"

namespace system_memory {
  
// Returns an estimation of free stack memory in bytes. Does not
// include free memory blocks in the heap.
extern uint32_t estimateStackFreeSize();

}  // namespace system_memory

#endif  // SYSTEM_MEMORY_H


