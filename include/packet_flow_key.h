#ifndef PACKET_FLOW_KEY_H
#define PACKET_FLOW_KEY_H

#include <arpa/inet.h>
#include <netinet/ip.h>
#include "packet_direction.h"  // Include this for PacketDirection enum
#include "packet.h"

// Function declaration to get the flow key as a string
char* get_flow_key(Packet *packet, PacketDirection direction);

#endif // PACKET_FLOW_KEY_H
