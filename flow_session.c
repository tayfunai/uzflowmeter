// flow_session.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include "include/flow_session.h"
#include "include/packet_time.h"

// Function to initialize the FlowSession
FlowSession* init_flow_session(int initial_capacity) {
    FlowSession *session = (FlowSession *) malloc(sizeof(FlowSession));
    session->flows = (Flows *) malloc(sizeof(Flows) * initial_capacity);  // Allocate memory for Flows array
    session->capacity = initial_capacity;
    session->flow_count = 0;
    session->packet_count = 0;
    return session;
}


void add_flow(FlowSession *session, const char *flow_key, Flow *flow, int count) {
    // Check if the capacity is full, if so, double the size of the array
    if (session->flow_count == session->capacity) {
        int new_capacity = session->capacity * 2;
        Flows *flows = (Flows *) realloc(session->flows, sizeof(Flows) * new_capacity);
        if (flows == NULL) {
            // Handle memory allocation failure
            fprintf(stderr, "Error: Unable to allocate memory for flows array\n");
            return;
        }
        session->flows = flows;
        session->capacity = new_capacity;
    }

    // Allocate memory for the key and copy it
    session->flows[session->flow_count].key = strdup(flow_key);
    if (session->flows[session->flow_count].key == NULL) {
        // Handle memory allocation failure for the key
        fprintf(stderr, "Error: Unable to allocate memory for flow key\n");
        return;
    }

    // Set the flow and use the passed count
    session->flows[session->flow_count].flow = flow;
    session->flows[session->flow_count].count = count;  // Use the count passed as parameter

    // Increment the flow count
    session->flow_count++;
}

int delete_flow_by_key(FlowSession *session, const char *key) {
    for (int i = 0; i < session->flow_count; i++) {
        if (strcmp(session->flows[i].key, key) == 0) {
            // Free the memory for the flow's key
            free(session->flows[i].key);

            // Free the Flow structure, including all nested allocations
            if (session->flows[i].flow != NULL) {
                free_flow(session->flows[i].flow);
            }

            // Shift the remaining flows to fill the gap
            for (int j = i; j < session->flow_count - 1; j++) {
                session->flows[j] = session->flows[j + 1];
            }

            // Decrease the flow count
            session->flow_count--;

            return 1;  // Return success
        }
    }

    return 0;  // No matching flow found
}


// Free the flow session
void free_flow_session(FlowSession *session) {
    if (session == NULL) {
        return;
    }

    // Free each flow in the flows array
    for (int i = 0; i < session->flow_count; i++) {
        if (session->flows[i].key != NULL) {
            free(session->flows[i].key);
        }

        if (session->flows[i].flow != NULL) {
            free_flow(session->flows[i].flow);  // Free each Flow object
        }
    }

    // Free the flows array itself
    if (session->flows != NULL) {
        free(session->flows);
    }

    // Free the session structure itself
    free(session);
}

Flow* get_flow(FlowSession *session, const char *key, int count) {
    for (int i = 0; i < session->flow_count; i++) {
        if (strcmp(session->flows[i].key, key) == 0 && session->flows[i].count == count) {
            return session->flows[i].flow;  // Return the matching flow
        }
    }
    return NULL;  // Return NULL if no match is found
}

// Function to detect the direction of a packet based on the src_ip and dest_ip with respect to HOST_IP
// PacketDirection detect_packet_direction(const Packet *packet) {
//     // If the packet is coming from the host (src_ip matches HOST_IP), it's FORWARD
//     if (strcmp(packet->src_ip, HOST_IP) == 0) {
//         return FORWARD;
//     }

//     // If the packet is destined for the host (dest_ip matches HOST_IP), it's REVERSE
//     if (strcmp(packet->dest_ip, HOST_IP) == 0) {
//         return REVERSE;
//     }

//     // Default to FORWARD if neither match (depending on your application's logic)
//     return FORWARD;
// }
// Function to get a list of keys from the flows in the session
char** get_flow_keys(FlowSession *session) {
    // Allocate memory for the list of keys
    char **keys = malloc(session->flow_count * sizeof(char *));
    if (keys == NULL) {
        fprintf(stderr, "Error allocating memory for keys\n");
        return NULL;
    }

    // Iterate over the flows and copy the keys
    for (int i = 0; i < session->flow_count; i++) {
        keys[i] = strdup(session->flows[i].key);  // Duplicate the key to avoid reference issues
        if (keys[i] == NULL) {
            fprintf(stderr, "Error duplicating key\n");
            // Free already allocated keys in case of error
            for (int j = 0; j < i; j++) {
                free(keys[j]);
            }
            free(keys);
            return NULL;
        }
    }

    return keys;
}
Flow* get_flow_by_its_key(FlowSession *session, const char *key) {
    for (int i = 0; i < session->flow_count; i++) {
        if (strcmp(session->flows[i].key, key) == 0) {
            return session->flows[i].flow;  // Return the matching flow
        }
    }
    return NULL;  // Return NULL if no match is found
}

int find_flow_by_key(FlowSession *session, const char *key) {
    for (int i = 0; i < session->flow_count; i++) {
        if (strcmp(session->flows[i].key, key) == 0) {
            return i;  // Return the index of the flow with the matching key
        }
    }
    return -1;  // Return -1 if the flow is not found
}

void on_packet_received(FlowSession *session, Packet *packett, const struct pcap_pkthdr *pkthdr, const u_char *packet){
    int count = 0;
    PacketDirection direction = FORWARD;
    parse_packet(pkthdr, packet, packett);  // Parse the packet to fill the `packett` structure
    // printf("Mebni kechir jonvorbek %f\n", packett->time);
    char *flow_key;
    if (packett->protocol == IPPROTO_TCP || packett->protocol == IPPROTO_UDP) {

        flow_key = get_flow_key(packett, direction);

        Flow *flow = get_flow(session, flow_key, count);

        session->packet_count += 1;

        // If there is no forward flow with a count of 0
        if (flow == NULL) {  // Free the forward key if dynamically allocated
        direction = REVERSE;
            flow_key = get_flow_key(packett, direction);  // Get the reverse key
            flow = get_flow(session, flow_key, count);
            if(flow != NULL){
                add_packet_to_flow(flow, packett, direction);
            }
        }
        // Detect the packet direction
        // direction = detect_packet_direction(packett);
        direction = FORWARD;
        // If no flow exists, create a new flow
        if (flow == NULL) {
            flow = (Flow *) malloc(sizeof(Flow));  // Dynamically allocate memory for the flow
            if (flow == NULL) {
                perror("Failed to allocate memory for flow");
                exit(EXIT_FAILURE);  // Handle memory allocation failure
            }

            memset(flow, 0, sizeof(Flow));  // Initialize the new flow to avoid undefined behavior
            memcpy(flow->src_ip, packett->src_ip, sizeof(flow->src_ip));
            memcpy(flow->dest_ip, packett->dest_ip, sizeof(flow->dest_ip));            
            flow->src_port = packett->src_port;
            flow->dest_port = packett->dest_port;
            flow->protocol = packett->protocol;
            flow_key = get_flow_key(packett, direction);  // Get the flow key for the new flow
            add_packet_to_flow(flow, packett, direction);
            add_flow(session, flow_key, flow, count);  // Add the flow to the session
            // free(flow);
            
        } else if ((packett->time - flow->latest_timestamp) > EXPIRED_UPDATE) {
            // If the packet exists in the flow but the packet is sent
            // after too much of a delay, then it is part of a new flow.
            int expired = EXPIRED_UPDATE;

            // Logic to check and update flow based on the timestamp
            while ((packett->time - flow->latest_timestamp) > expired) {
                count++;
                expired += EXPIRED_UPDATE;

                flow = get_flow(session, flow_key, count);

                if (flow == NULL) {
                    flow = (Flow *) malloc(sizeof(Flow));  // Dynamically allocate memory for the new flow
                    if (flow == NULL) {
                        perror("Failed to allocate memory for flow");
                        exit(EXIT_FAILURE);
                    }
                    memset(flow, 0, sizeof(Flow));  // Initialize the new flow
                    memcpy(flow->src_ip, packett->src_ip, sizeof(flow->src_ip));
                    memcpy(flow->dest_ip, packett->dest_ip, sizeof(flow->dest_ip));            
                    flow->src_port = packett->src_port;
                    flow->dest_port = packett->dest_port;
                    flow->protocol = packett->protocol;
                    flow_key = get_flow_key(packett, direction);
                    add_flow(session, flow_key, flow, count);  // Add the flow to the session
                    // free(flow);
                    break;
                }
            }
        } else if (packett->flags & TH_FIN) {
            add_packet_to_flow(flow, packett, direction);
            garbage_collect(session, packett->time, packett);
            return;
            // printf("OMG");
        }
        add_packet_to_flow(flow, packett, direction);

        // printf("%f\t", get_duration(flow));
        // printf("Packet count  %d\n" ,session->packet_count);
        // if (get_duration(flow) > 20) {

        if (session->packet_count % GARBAGE_COLLECT_PACKETS == 0 || get_duration(flow) > 120) {
            // printf("  %d\n" ,session->packet_count);
            garbage_collect(session, packett->time, packett);  // Perform garbage collection
        }
    }
      // Free the dynamically allocated flow_key after use
}


void garbage_collect(FlowSession *session, double latest_time, Packet *packet) {
    char **keys = get_flow_keys(session);
    printf("Garbage Collection Began. Flows = {%d}\n", session->flow_count);

    if (keys != NULL) {
        for (int i = 0; i < session->flow_count; i++) {
            Flow *flow = get_flow_by_its_key(session, keys[i]);
            if (flow == NULL) {
                continue;  // Skip if flow is NULL
            }

            // Check if flow is expired
            if (latest_time - flow->latest_timestamp > EXPIRED_UPDATE || get_duration(flow) > 90) {
                get_data(flow, packet);
                delete_flow_by_key(session, keys[i]);  // Delete expired flow
            }
        }

        // Free the list of keys after use
        for (int i = 0; i < session->flow_count; i++) {
            free(keys[i]);
        }
        free(keys);
        printf("Garbage Collection Finished. Flows = {%d}\n", session->flow_count);

    }
}

