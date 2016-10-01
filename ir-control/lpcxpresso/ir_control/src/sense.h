// This module sense the on/off states of the AV and TV.

#ifndef SENSE_H
#define SENSE_H

namespace sense {

extern void setup();

extern void loop();

extern bool is_av_on();

extern bool is_tv_on();

}  // sense

#endif  // sense_H
