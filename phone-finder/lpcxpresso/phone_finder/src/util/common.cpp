
#include "util/common.h"
#include "USBserial.h"

namespace common {
  
USBSerial stdio(0x1f00, 0x2012, 0x0001, false);

}  // namespace common
