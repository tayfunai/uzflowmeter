#include <stdio.h>
#include <string.h>
#include "include/flow.h"
#include "include/packet_count.h"
#include "include/packet_time.h"

#define PROTOCOL_TCP 6
#define PROTOCOL_UDP 17


int get_payload(Packet *packet) {
    if (packet == NULL) {
        return 0; // Return 0 if the packet is NULL
    }

    // Check if the packet is TCP
    if (packet->protocol == PROTOCOL_TCP) {
        return packet->payload_size; // Return the TCP payload size
    }
    // Check if the packet is UDP
    else if (packet->protocol == PROTOCOL_UDP) {
        return packet->payload_size; // Return the UDP payload size
    }

    // If the protocol is neither TCP nor UDP, return 0
    return 0;
}


// Function to calculate the number of packets with payload
int has_payload(Flow *flow, PacketDirection packet_direction) {
    int count = 0;

    // Iterate through all packets in the flow
    for (int i = 0; i < flow->packets.count; ++i) {
        // Get the time of the first packet
        Pair pair = flow->packets.pairs[i];
        if ((pair.direction == packet_direction) &&
            get_payload(pair.packet) > 0) {
            count++;
        }
    }

    return count;
}

int get_total(Flow *flow, PacketDirection packet_direction) {
    int packets_count=0;
    for (int i = 0; i < flow->packets.count; ++i) {
        Pair pair = flow->packets.pairs[i];
        // If packet_direction is provided, check if it matches the current packet's direction
        if(packet_direction == BOTH){
            packets_count++;
        }else{
            if (pair.direction == packet_direction) {
                packets_count++;
            }
        }
        
    }

    return packets_count;
}

double get_rate_count(Flow *flow, PacketDirection packet_direction) {

    double duration = get_duration(flow);  // Get the duration of the flow (PacketTime)

    if (duration == 0) {
        return 0.0;
    } else {
        int total_packets = get_total(flow, packet_direction);  // Get total packets in the direction
        return total_packets / duration;  // Calculate rate as packets per second
    }
}

double get_down_up_ratio(Flow *flow) {
    // Get total packets in the forward direction (download)
    int forward_size = get_total(flow, FORWARD);

    // Get total packets in the reverse direction (upload)
    int backward_size = get_total(flow, REVERSE);

    // Calculate and return the download/upload ratio
    if (forward_size > 0) {
        double b, c;
        b = (double)backward_size;
        c = (double)forward_size;
        double ratio = b/c;
        return ratio;
    }
    return 0.0;
}