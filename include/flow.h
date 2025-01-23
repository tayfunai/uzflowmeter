#ifndef FLOW_H
#define FLOW_H

#include <stdlib.h>  // For memory management functions like malloc, free
#include <stddef.h>  // For size_t
#include "packet_direction.h"
#include "packet.h"  // Assuming this contains the definition for Packet structure

#define CLUMP_TIMEOUT 1.0
#define BULK_BOUND 4
#define ACTIVE_TIMEOUT 0.005
#define INITIAL_CAPACITY 10  // Initial capacity for the dynamic array

// Structure to store interarrival times
typedef struct {
    double *times;    // Dynamic array to store interarrival times
    size_t count;     // Current number of elements
    size_t capacity;  // Allocated size of the array
} FlowInterarrivalTime;

// Structure for Packet pairs
typedef struct {
    Packet *packet;          // Changed to Packet pointer
    PacketDirection direction;
} Pair;

typedef struct {
    Pair *pairs;
    size_t count;
    size_t capacity;
} Packets;

// Structures for Active and Idle periods
typedef struct {
    double *active;
    int count;
    size_t capacity;
} Active;

typedef struct {
    double *idle;
    size_t count;
    size_t capacity;
} Idle;

// Flow structure
typedef struct {
    char src_ip[16];
    char dest_ip[16];
    int src_port;
    int dest_port;

    Packets packets;
    FlowInterarrivalTime flow_interarrival_time;
    uint8_t protocol;
    int init_window_size[2];

    double latest_timestamp;
    double start_timestamp;

    double start_active;
    double last_active;
    Active active;
    Idle idle;

    // Forward bulk properties
    double forward_bulk_last_timestamp;
    double forward_bulk_start_tmp;
    int forward_bulk_count;
    int forward_bulk_count_tmp;
    double forward_bulk_duration;
    int forward_bulk_packet_count;
    int forward_bulk_size;
    int forward_bulk_size_tmp;

    // Backward bulk properties
    double backward_bulk_last_timestamp;
    double backward_bulk_start_tmp;
    int backward_bulk_count;
    int backward_bulk_count_tmp;
    double backward_bulk_duration;
    int backward_bulk_packet_count;
    int backward_bulk_size;
    int backward_bulk_size_tmp;
} Flow;

void initializeFlowInterarrivalTime(FlowInterarrivalTime *flowTimes);
void appendFlowInterarrivalTime(FlowInterarrivalTime *flowTimes, double time);
void freeFlowInterarrivalTtime(FlowInterarrivalTime *flowTimes);


void initializePair(Pair *pair, Packet *packet, PacketDirection direction); // Changed to accept Packet pointer
void initializePackets(Packets *packets, size_t initialCapacity);
void appendPair(Packets *packets, Packet *packet, PacketDirection direction); // Changed to accept Packet pointer
void freePackets(Packets *packets);

void initializeActive(Active *a, size_t initialCapacity);
void initializeIdle(Idle *i, size_t initialCapacity);
void appendActive(Active *a, double value);
void appendIdle(Idle *i, double value);
void freeActive(Active *a);
void freeIdle(Idle *i);

void add_packet_to_flow(Flow *flow, Packet *packet, PacketDirection direction); // Changed to accept Packet pointer
void update_subflow(Flow *flow, Packet *packet);
void update_active_idle(Flow *flow, double current_time);
void update_flow_bulk(Flow *flow, Packet *packet, PacketDirection direction);   // Changed to accept Packet pointer
double get_flow_duration(Flow *flow);
void get_data(Flow *flow, Packet *packet);
void free_flow(Flow *flow);

#endif // FLOW_H