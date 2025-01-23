#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "include/packet_time.h"



// int* direction_list(Flow* flow) {
//     // Allocate memory to store the directions (max 50 packets)
//     int* direction_list = (int*)malloc(50 * sizeof(int));
//     if (direction_list == NULL) {
//         printf("Memory allocation failed!\n");
//         exit(1);
//     }

//     // Loop through the first 50 packets or as many packets as available
//     int limit = (flow->packet_count < 50) ? flow->packet_count : 50;
//     for (int i = 0; i < limit; i++) {
//         direction_list[i] = pairr.direction;  // Store the direction of the packet
//     }

//     // If there are fewer than 50 packets, fill the rest with -1 (indicating no packet)
//     for (int i = limit; i < 50; i++) {
//         direction_list[i] = -1;
//     }

//     return direction_list;  // Return the list of directions
// }


int get_bytes(Flow* flow) {
    int total_bytes = 0;

    // Loop through all packets in the flow
    for (int i = 0; i < flow->packets.count; i++) {
        Pair pairr = flow->packets.pairs[i];
        total_bytes += pairr.packet->payload_size;
        // total_bytes += strlen(pairr.packet);  // Add the length of the packet data
    }

    return total_bytes;  // Return the total number of bytes
}


double get_rate_bytes(Flow* flow) {
    double duration = get_duration(flow);  // Get the duration of the flow

    if (duration == 0) {
        return 0.0;  // If the duration is 0, return a rate of 0 to avoid division by zero
    }

    int total_bytes = get_bytes(flow);     // Get the total number of bytes transferred
    double rate = total_bytes / duration;  // Calculate the rate (bytes/sec)

    return rate;  // Return the rate
}

int get_bytes_sent(Flow* flow) {
    int total_bytes_sent = 0;

    // Loop through all packets in the flow
    for (int i = 0; i < flow->packets.count; i++) {
        // Check if the packet direction is FORWARD (sent from the machine)
        Pair pairr = flow->packets.pairs[i];
        if (pairr.direction == FORWARD) {
            total_bytes_sent += pairr.packet->payload_size;  // Add the length of the packet data
        }
    }

    return total_bytes_sent;  // Return the total number of bytes sent
}

double get_sent_rate(Flow* flow) {
    int sent_bytes = get_bytes_sent(flow);  // Get the total number of bytes sent (in FORWARD direction)
    double duration = get_duration(flow);   // Get the duration of the flow

    if (duration == 0) {
        return -1.0;  // If the duration is 0, return a rate of -1 to indicate no valid rate
    }

    double rate = sent_bytes / duration;  // Calculate the rate (bytes/sec)
    return rate;  // Return the rate
}

int get_bytes_received(Flow* flow) {
    int total_bytes_received = 0;

    // Loop through all packets in the flow
    for (int i = 0; i < flow->packets.count; i++) {
        Pair pairr = flow->packets.pairs[i];

        // Check if the packet direction is REVERSE (received by the machine)
        if (pairr.direction == REVERSE) {
            total_bytes_received += pairr.packet->payload_size;  // Add the length of the packet data
        }
    }

    return total_bytes_received;  // Return the total number of bytes received
}

double get_received_rate(Flow* flow) {
    int received_bytes = get_bytes_received(flow);  // Get the total number of bytes received (in REVERSE direction)
    double duration = get_duration(flow);           // Get the duration of the flow

    if (duration == 0) {
        return -1.0;  // If the duration is 0, return a rate of -1 to indicate no valid rate
    }

    double rate = received_bytes / duration;  // Calculate the rate (bytes/sec)
    return rate;  // Return the rate
}

int header_size(int ihl) {
    return ihl * 4;  // IHL is in 32-bit words, multiply by 4 to get the size in bytes
}

int get_forward_header_bytes(Flow* flow) {
    int total_header_bytes = 0;

    // Loop through all packets in the flow
    for (int i = 0; i < flow->packets.count; i++) {
        Pair pairr = flow->packets.pairs[i];
        // Check if the packet direction is FORWARD (sent in the same direction as the flow)
        if (pairr.direction == FORWARD) {
            total_header_bytes += pairr.packet->header_size;  // Add the header size of the packet
        }
    }

    return total_header_bytes;  // Return the total number of header bytes
}

// Function to calculate the rate of bytes being forwarded in the flow (bytes/sec)
double get_forward_rate(Flow* flow) {
    int forward_bytes = get_forward_header_bytes(flow);  // Get the total number of forward header bytes
    double duration = get_duration(flow);                // Get the duration of the flow

    if (duration > 0) {
        return forward_bytes / duration;  // Calculate the rate (bytes/sec) if duration is greater than 0
    } else {
        return -1.0;  // Return -1 if the duration is 0 or negative
    }
}

int _header_size(Packet* packet) {
    if (packet->protocol == IPPROTO_TCP) {
        return packet->ihl * 4;  // If the packet contains TCP, calculate IP header size (IHL * 4 bytes)
    } else {
        return 8;  // If the packet does not contain TCP, return a default size of 8 bytes
    }
}
        

int get_reverse_header_bytes(Flow* flow) {
    if (flow->packets.count == 0) {
        return 0;  // Return 0 if there are no packets
    }

    int total_header_bytes = 0;

    // Loop through all packets in the flow
    for (int i = 0; i < flow->packets.count; i++) {
        Pair pairr = flow->packets.pairs[i];
        // Check if the packet direction is REVERSE
        if (pairr.direction == REVERSE) {
            total_header_bytes += pairr.packet->header_size;  // Add the header size of the packet
        }
    }

    return total_header_bytes;  // Return the total number of header bytes in the reverse direction
}


int get_min_forward_header_bytes(Flow* flow) {
    if (flow->packets.count == 0) {
        return 0;  // Return 0 if there are no packets
    }


    int min_header_bytes = flow->packets.pairs[0].packet->header_size;  // Initialize with a large number
    
    // If no FORWARD packets were found, return 0
    if (flow->packets.pairs == NULL) {
        return 0;
    }

    // Loop through all packets in the flow
    for (int i = 0; i < flow->packets.count; i++) {
        Pair pairr = flow->packets.pairs[i];
        // Check if the packet direction is FORWARD
        if (pairr.direction == FORWARD) {
            int current_header_size = pairr.packet->header_size;
            if (current_header_size < min_header_bytes) {
                min_header_bytes = current_header_size;  // Update the minimum header size
            }
        }
    }
    return min_header_bytes;  // Return the minimum header size
}


double get_reverse_rate(Flow *flow) {
    double reverse = get_reverse_header_bytes(flow);
    double duration = get_duration(flow);
    
    if (duration == 0) {
        return -1.0; // If duration is 0, return -1 as per the logic
    } else {
        return reverse / duration; // Calculate reverse rate (bytes/sec)
    }
}

double get_header_in_out_ratio(Flow *flow) {
    double reverse_header_bytes = get_reverse_header_bytes(flow);
    double forward_header_bytes = get_forward_header_bytes(flow);
    
    if (reverse_header_bytes == 0) {
        return -1.0; // To avoid division by zero, return -1 if reverse bytes are 0
    } else {
        return forward_header_bytes / reverse_header_bytes; // Calculate the ratio
    }
}

// int get_initial_ttl(Flow *flow) {
//     if (flow->packets.count == 0) {
//         // No packets in flow
//         return -1;
//     }
//     Pair pairr = flow->packets.pairs[i];

//     const u_char *packet = flow->packets[0].packet; // Get the first packet
//     struct ip *ip_header;

//     // Skip the Ethernet header (assuming Ethernet here, 14 bytes)
//     ip_header = (struct ip *)(packet + 14);

//     // Return the TTL from the IP header
//     return ip_header->ip_ttl;
// }

double get_bytes_per_bulk(Flow *flow, PacketDirection packet_direction) {
    if (packet_direction == FORWARD) {
        if (flow->forward_bulk_count != 0) {
            return flow->forward_bulk_size / flow->forward_bulk_count;
        }
    } else {
        if (flow->backward_bulk_count != 0) {
            return flow->backward_bulk_size / flow->backward_bulk_count;
        }
    }
    return 0.0;
}

double get_packets_per_bulk(Flow *flow, PacketDirection packet_direction) {
    if (packet_direction == FORWARD) {
        if (flow->forward_bulk_count != 0) {
            return (double)flow->forward_bulk_packet_count / flow->forward_bulk_count;
        }
    } else {
        if (flow->backward_bulk_count != 0) {
            return (double)flow->backward_bulk_packet_count / flow->backward_bulk_count;
        }
    }
    return 0.0;
}

double get_bulk_rate(Flow *flow, PacketDirection packet_direction) {
    if (packet_direction == FORWARD) {
        if (flow->forward_bulk_count != 0) {
            return flow->forward_bulk_size / flow->forward_bulk_duration;
        }
    } else {
        if (flow->backward_bulk_count != 0) {
            return flow->backward_bulk_size / flow->backward_bulk_duration;
        }
    }
    return 0.0;
}

