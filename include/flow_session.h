// flow_session.h
#ifndef FLOW_SESSION_H
#define FLOW_SESSION_H

#include <pcap.h>
#include "flow.h"
#include "packet.h"
#include "packet_flow_key.h"

// #define HOST_IP "192.168.1.5"
#define EXPIRED_UPDATE 40
#define GARBAGE_COLLECT_PACKETS 10000

// Define the structure for holding flows in the session
typedef struct {
    char *key;    // Flow key (source IP, destination IP, source port, destination port)
    int count;
    Flow *flow;           // Pointer to the Flow object
} Flows;

typedef struct FlowSession {
    Flows *flows;
    int flow_count;
    int packet_count;
    int capacity;
} FlowSession;


void pack_flow_key(char *packed_key, const char *src_ip, const char *dest_ip, uint16_t src_port, uint16_t dest_port, uint8_t protocol);
void on_packet_received(FlowSession *session, Packet *packett, const struct pcap_pkthdr *pkthdr, const u_char *packet);
FlowSession* init_flow_session(int initial_capacity);
void add_flow(FlowSession *session, const char *flow_key, Flow *flow, int count);
void free_flow_session(FlowSession *session);
Flow* get_flow(FlowSession *session, const char *key, int count);
Flow* get_flow_by_its_key(FlowSession *session, const char *key);
PacketDirection detect_packet_direction(const Packet *packet);
int delete_flow_by_key(FlowSession *session, const char *key);

void garbage_collect(FlowSession *session, double latest_time, Packet *packet);

char** get_flow_keys(FlowSession *session);
#endif // FLOW_SESSION_H
