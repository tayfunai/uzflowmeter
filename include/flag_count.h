#ifndef FLAG_COUNT_H
#define FLAG_COUNT_H

#include "flow.h"
#include "packet_direction.h"


// Function declaration for checking if a specific flag exists in the flow
int flag_counter(Flow *flow, uint8_t flag, PacketDirection packet_direction);

#endif // FLAG_COUNT_H
