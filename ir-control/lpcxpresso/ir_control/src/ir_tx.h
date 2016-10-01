// This module is responsible for sending the AV IR commands.

#ifndef IR_TX_H
#define IR_TX_H

namespace ir_tx {

extern void setup();

extern void start_tx(int packets);

extern int tx_packets_pending();

extern void dump_state();

}  // ir_tx

#endif  // IR_TX_H
