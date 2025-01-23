#ifndef FLOW_METRICS_H
#define FLOW_METRICS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <pcap.h>
#include <netinet/ip.h>


// Function declarations
int* direction_list(Flow* flow);
int get_bytes(Flow* flow);
double get_rate_bytes(Flow* flow);
int get_bytes_sent(Flow* flow);
double get_sent_rate(Flow* flow);
int get_bytes_received(Flow* flow);
double get_received_rate(Flow* flow);
int header_size(int ihl);
int get_forward_header_bytes(Flow* flow);
double get_forward_rate(Flow* flow);
int _header_size(Packet* packet);
int get_reverse_header_bytes(Flow* flow);
int get_min_forward_header_bytes(Flow* flow);
double get_reverse_rate(Flow *flow);
double get_header_in_out_ratio(Flow *flow);
int get_initial_ttl(Flow *flow);
double get_bytes_per_bulk(Flow *flow, PacketDirection packet_direction);
double get_packets_per_bulk(Flow *flow, PacketDirection packet_direction);
double get_bulk_rate(Flow *flow, PacketDirection packet_direction);

// External function (you would need to implement this)
double get_duration(Flow *flow);

#endif // FLOW_METRICS_H
