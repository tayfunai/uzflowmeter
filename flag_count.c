#include "include/flag_count.h"


int flag_counter(Flow *flow, uint8_t flag, PacketDirection packet_direction) {
    int count = 0;

    // Iterate through the packets in the flow
    for (int i = 0; i < flow->packets.count; i++) {
        Pair pair = flow->packets.pairs[i];

        // Check if the packet is NULL
        if (pair.packet == NULL) {
            fprintf(stderr, "Warning: Packet at index %d is NULL\n", i);
            continue;  // Skip this packet if it's NULL
        }

        // Filter out packets by protocol: if it's UDP, continue without counting
        if (pair.packet->protocol == IPPROTO_UDP) {
            continue;  // Skip UDP packets
        }

        // Check if the packet direction matches, if provided
        if (packet_direction == BOTH){
            if ((pair.packet->flags & flag) == flag) {
                count++;
            }
        } else if (pair.direction == packet_direction) {
            // Check if the flag is present in the packet's flags
            if ((pair.packet->flags & flag) == flag) {
                count++;
            }
        }
    }

    return count;
}