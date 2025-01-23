#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <string.h>
#include "include/packet_direction.h"  // Ensure to include your PacketDirection enum definition
#include "include/packet.h"

#define FLOW_KEY_SIZE 100  // Define a constant for the buffer size



char* get_flow_key(Packet *packet, PacketDirection direction) {
    // Allocate a static char buffer for the flow key (this must be large enough to hold the resulting string)
    static char flow_key[FLOW_KEY_SIZE];

    // Based on the direction, print either the forward or reverse flow key
    if (direction == FORWARD) {
        snprintf(flow_key, FLOW_KEY_SIZE, "%s_%s_%u_%u_%u", 
                 packet->src_ip, 
                 packet->dest_ip, 
                 packet->src_port,   // Convert source port to host byte order
                 packet->dest_port,   // Convert destination port to host byte order
                 packet->protocol);
    } else if (direction == REVERSE) {
        snprintf(flow_key, FLOW_KEY_SIZE, "%s_%s_%u_%u_%u", 
                 packet->dest_ip, 
                 packet->src_ip, 
                 packet->dest_port,  // Reverse flow: use dest as src port
                 packet->src_port,   // Reverse flow: use src as dest port
                 packet->protocol);
    }

    // Return the formatted flow key string
    return flow_key;
}