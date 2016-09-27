#ifndef IR_TX_H
#define IR_TX_H

namespace ir_tx {

extern void setup();

extern void start_tx(int packets);

extern int get_packets_left();

extern void dump_state();

}  // ir_tx

#endif  // IR_TX_H
