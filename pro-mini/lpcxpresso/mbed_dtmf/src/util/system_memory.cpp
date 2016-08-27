#include "util/system_memory.h"

// This can be found in the linker's map. Presumably it points to the
// top of the heap.
extern uint32_t __end_of_heap;

namespace system_memory {

uint32_t estimateStackFreeSize() {
  uint32_t dummy;
  return reinterpret_cast<uint32_t>(&dummy) - __end_of_heap;
}

}  // namespace system_memory

