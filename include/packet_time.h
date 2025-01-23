#ifndef PACKET_TIME_H
#define PACKET_TIME_H


#include <time.h>  // For time_t
#include "packet_direction.h"
#include "flow.h"

// Define MAX_PACKETS
#define MAX_PACKETS 1000


// Function declarations
double* _get_packet_times(Flow *flow);
double* get_packet_iat(Flow *flow, PacketDirection packet_direction, int *iat_size);
double* relative_time_list(Flow *flow);
char* get_time_stamp(Flow *flow);
double get_duration(Flow *flow);
double get_std(Flow *flow);
double get_mean(Flow *flow);
double get_median(Flow *flow);
double get_mode(Flow *flow);
double get_skew(Flow *flow);
double get_skew2(Flow *flow);
double get_cov(Flow *flow);
int compare_doubles(const void *a, const void *b);

#endif /* PACKET_TIME_H */
