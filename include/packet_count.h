#ifndef PACKET_COUNT_H
#define PACKET_COUNT_H

#include "flow.h"
#include <stdio.h>
#include <string.h>
#include "packet_direction.h"


int get_total(Flow *flow, PacketDirection packet_direction);
double get_rate_count(Flow *flow, PacketDirection packet_direction);
double get_down_up_ratio(Flow *flow);
int get_payload(Packet *packet);
int has_payload(Flow *flow, PacketDirection packet_direction);

#endif // FLOW_H
