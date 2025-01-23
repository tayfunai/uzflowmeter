#include <math.h>
#include "include/flow.h"
#include <stdio.h>
#include <stdlib.h>
#include "include/packet_count.h"
#include <netinet/tcp.h>  // For TCP header
#include <json-c/json.h>
#include "include/packet_time.h"
#include "include/flow_bytes.h"
#include "include/flow_session.h"
#include "include/packet_direction.h"
#include "include/packet_length.h"
#include "include/flag_count.h"
#include "include/utils.h"
#include <curl/curl.h>
#include "include/socket.h"


void initializeFlowInterarrivalTime(FlowInterarrivalTime *flowTimes) {
    flowTimes->times = (double *)malloc(INITIAL_CAPACITY * sizeof(double));
    if (flowTimes->times == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);  // Handle memory allocation failure
    }
    flowTimes->count = 0;
    flowTimes->capacity = INITIAL_CAPACITY;
}


void appendFlowInterarrivalTime(FlowInterarrivalTime *flowTimes, double newTime) {
    if (flowTimes->count == flowTimes->capacity) {
        flowTimes->capacity *= 2;  // Double the capacity
        double *newArray = (double *)realloc(flowTimes->times, flowTimes->capacity * sizeof(double));
        if (newArray == NULL) {
            perror("Failed to reallocate memory");
            exit(EXIT_FAILURE);  // Handle memory allocation failure
        }
        flowTimes->times = newArray;
    }
    flowTimes->times[flowTimes->count] = newTime;
    flowTimes->count++;
}
void freeFlowInterarrivalTtime(FlowInterarrivalTime *flowTimes) {
    if (flowTimes == NULL) return;
    free(flowTimes->times);
}


void initializePair(Pair *pair, Packet *packet, PacketDirection direction) {
    // Allocate memory for a new packet
    pair->packet = (Packet *)malloc(sizeof(Packet));
    if (pair->packet == NULL) {
        fprintf(stderr, "Memory allocation failed for new packet\n");
        exit(1);
    }
    // Copy the content of the original packet to the new packet
    memcpy(pair->packet, packet, sizeof(Packet));

    pair->direction = direction;
}


void initializePackets(Packets *packets, size_t initialCapacity) {
    packets->pairs = (Pair *)malloc(initialCapacity * sizeof(Pair));
    if (packets->pairs == NULL) {
        fprintf(stderr, "Memory allocation failed for pairs\n");
        exit(1);
    }
    packets->count = 0;
    packets->capacity = initialCapacity;
}
void appendPair(Packets *packets, Packet *packet, PacketDirection direction) {
    if (packets->count >= packets->capacity) {
        packets->capacity *= 2;
        packets->pairs = (Pair *)realloc(packets->pairs, packets->capacity * sizeof(Pair));
        if (packets->pairs == NULL) {
            fprintf(stderr, "Memory reallocation failed for pairs\n");
            exit(1);
        }
    }
    
    // Initialize the new pair with the packet and direction
    initializePair(&packets->pairs[packets->count], packet, direction);

    // Increment the count of packets
    packets->count++;


}


void freePackets(Packets *packets) {
    if (packets == NULL) return;

    for (size_t i = 0; i < packets->count; i++) {
        if (packets->pairs[i].packet != NULL) {
            free(packets->pairs[i].packet);
        }
    }
    free(packets->pairs);

}

// Initialize and manage Active and Idle arrays
void initializeActive(Active *a, size_t initialCapacity) {
    a->active = (double *)malloc(initialCapacity * sizeof(double));
    if (a->active == NULL) {
        fprintf(stderr, "Memory allocation failed for active array\n");
        exit(1);
    }
    a->count = 0;
    a->capacity = initialCapacity;
}

void appendActive(Active *a, double value) {
    if (a->count >= a->capacity) {
        a->capacity *= 2;
        a->active = (double *)realloc(a->active, a->capacity * sizeof(double));
        if (a->active == NULL) {
            fprintf(stderr, "Memory reallocation failed for active array\n");
            exit(1);
        }
    }
    a->active[a->count++] = value;
}

void freeActive(Active *a) {
    if (a == NULL) return;
    free(a->active);
}

void initializeIdle(Idle *i, size_t initialCapacity) {
    i->idle = (double *)malloc(initialCapacity * sizeof(double));
    if (i->idle == NULL) {
        fprintf(stderr, "Memory allocation failed for idle array\n");
        exit(1);
    }
    i->count = 0;
    i->capacity = initialCapacity;
}

void appendIdle(Idle *i, double value) {
    if (i->count >= i->capacity) {
        i->capacity *= 2;
        i->idle = (double *)realloc(i->idle, i->capacity * sizeof(double));
        if (i->idle == NULL) {
            fprintf(stderr, "Memory reallocation failed for idle array\n");
            exit(1);
        }
    }
    i->idle[i->count++] = value;
}

void freeIdle(Idle *i) {
    if (i == NULL) return;
    free(i->idle);
}

void free_flow(Flow *flow) {
    if (flow == NULL) {
        return;
    }

    // Free Packets
    freePackets(&(flow->packets));

    // Free FlowInterarrivalTime
    freeFlowInterarrivalTtime(&(flow->flow_interarrival_time));

    // Free Active and Idle arrays
    freeActive(&(flow->active));
    freeIdle(&(flow->idle));

    // Free the Flow structure itself
    free(flow);
}


void send_json_to_clickhouse(const char *json_str) {
    CURL *curl;
    CURLcode res;

    // Initialize cURL globally
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    
    if (curl) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        // Set the ClickHouse endpoint (with the username and password)
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8123/?query=INSERT%20INTO%20cicflowmeter.test%20FORMAT%20JSONEachRow");

        // Set HTTP headers
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Set the POST data (your JSON string)
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str);

        // Add username and password for authentication
        curl_easy_setopt(curl, CURLOPT_USERNAME, "default");  // Username
        curl_easy_setopt(curl, CURLOPT_PASSWORD, "admin");    // Password

        // Perform the request
        res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

        // Cleanup
        curl_slist_free_all(headers);  // Free headers
        curl_easy_cleanup(curl);
    }

    // Cleanup cURL globally
    curl_global_cleanup();
}

// Extract detrailed flow features
void get_data(Flow *flow, Packet *packet){
    
    // Create a new JSON object
    struct json_object *json_obj = json_object_new_object();
    // PacketDirection direction = detect_packet_direction(packet);

    // Add IP information
    json_object_object_add(json_obj, "src_ip", json_object_new_string(flow->src_ip));
    json_object_object_add(json_obj, "dst_ip", json_object_new_string(flow->dest_ip));

    // // Add port information
    json_object_object_add(json_obj, "src_port", json_object_new_int(flow->src_port));
    json_object_object_add(json_obj, "dst_port", json_object_new_int(flow->dest_port));

    // Add MAC information
    // json_object_object_add(json_obj, "src_mac", json_object_new_string(packet->src_mac));
    // json_object_object_add(json_obj, "dst_mac", json_object_new_string(packet->dst_mac));

    // Add protocol information
    json_object_object_add(json_obj, "protocol", json_object_new_int(flow->protocol));

    //Basic information from packet times

    json_object_object_add(json_obj, "timestamp", json_object_new_string((get_time_stamp(flow))));
    json_object_object_add(json_obj, "flow_duration", json_object_new_double((get_duration(flow)) * 1e6));
    json_object_object_add(json_obj, "flow_byts_s", json_object_new_double(get_rate_bytes(flow)));
    json_object_object_add(json_obj, "flow_pkts_s", json_object_new_double(get_rate_count(flow, BOTH)));
    json_object_object_add(json_obj, "fwd_pkts_s", json_object_new_double(get_rate_count(flow, FORWARD)));
    json_object_object_add(json_obj, "bwd_pkts_s", json_object_new_double(get_rate_count(flow, REVERSE)));
    //Count total packets by direction
    json_object_object_add(json_obj, "tot_fwd_pkts", json_object_new_int64(get_total(flow, FORWARD)));
    json_object_object_add(json_obj, "tot_bwd_pkts", json_object_new_int64(get_total(flow, REVERSE)));
    //Statistical info obtained from Packet lengths
    json_object_object_add(json_obj, "totlen_fwd_pkts", json_object_new_int64(get_total_len(flow, FORWARD)));
    json_object_object_add(json_obj, "totlen_bwd_pkts", json_object_new_int64(get_total_len(flow, REVERSE)));
    json_object_object_add(json_obj, "fwd_pkt_len_max", json_object_new_double(get_max(flow, FORWARD)));
    json_object_object_add(json_obj, "fwd_pkt_len_min", json_object_new_double(get_min(flow, FORWARD)));
    json_object_object_add(json_obj, "fwd_pkt_len_mean", json_object_new_double(get_mean_len(flow, FORWARD)));
    json_object_object_add(json_obj, "fwd_pkt_len_std", json_object_new_double(get_std_len(flow, FORWARD)));
    json_object_object_add(json_obj, "bwd_pkt_len_max", json_object_new_double(get_max(flow, REVERSE)));
    json_object_object_add(json_obj, "bwd_pkt_len_min", json_object_new_double(get_min(flow, REVERSE)));
    json_object_object_add(json_obj, "bwd_pkt_len_mean", json_object_new_double(get_mean_len(flow, REVERSE)));
    json_object_object_add(json_obj, "bwd_pkt_len_std", json_object_new_double(get_std_len(flow, REVERSE)));
    json_object_object_add(json_obj, "pkt_len_max", json_object_new_double(get_max(flow, BOTH)));
    json_object_object_add(json_obj, "pkt_len_min", json_object_new_double(get_min(flow, BOTH)));
    json_object_object_add(json_obj, "pkt_len_mean", json_object_new_double(get_mean_len(flow, BOTH)));
    json_object_object_add(json_obj, "pkt_len_std", json_object_new_double(get_std_len(flow, BOTH)));
    json_object_object_add(json_obj, "pkt_len_var", json_object_new_double(get_var_len(flow, BOTH)));
    json_object_object_add(json_obj, "fwd_header_len", json_object_new_int(get_forward_header_bytes(flow)));
    json_object_object_add(json_obj, "bwd_header_len", json_object_new_int(get_reverse_header_bytes(flow)));
    json_object_object_add(json_obj, "fwd_seg_size_min", json_object_new_int(get_min_forward_header_bytes(flow)));
    json_object_object_add(json_obj, "fwd_act_data_pkts", json_object_new_int(has_payload(flow, FORWARD)));

    int iat_size;
    int iat_size_bwd;

    double *packet_iat = get_packet_iat(flow, FORWARD, &iat_size);
    double *packet_iat_bwd = get_packet_iat(flow, REVERSE, &iat_size_bwd);

    // Flows Interarrival Time
    Statistics flow_stats = get_flow_interarrival_statistics(&flow->flow_interarrival_time);
    Statistics forward_iat = get_statistics(packet_iat, iat_size);
    Statistics backward_iat = get_statistics(packet_iat_bwd, iat_size_bwd);
    Statistics idle_stat = get_idle_statistics(&flow->idle);
    Statistics active_stat = get_active_statistics(&flow->active);

    // Flows Interarrival Time 14
    json_object_object_add(json_obj, "flow_iat_mean", json_object_new_int(flow_stats.mean));
    json_object_object_add(json_obj, "flow_iat_max", json_object_new_int(flow_stats.max));
    json_object_object_add(json_obj, "flow_iat_min", json_object_new_int(flow_stats.min));
    json_object_object_add(json_obj, "flow_iat_std", json_object_new_int(flow_stats.std));

    json_object_object_add(json_obj, "fwd_iat_tot", json_object_new_int(forward_iat.total));
    json_object_object_add(json_obj, "fwd_iat_max", json_object_new_int(forward_iat.max));
    json_object_object_add(json_obj, "fwd_iat_min", json_object_new_int(forward_iat.min));
    json_object_object_add(json_obj, "fwd_iat_mean", json_object_new_int(forward_iat.mean));
    json_object_object_add(json_obj, "fwd_iat_std", json_object_new_int(forward_iat.std));

    json_object_object_add(json_obj, "bwd_iat_tot", json_object_new_int(backward_iat.total));
    json_object_object_add(json_obj, "bwd_iat_max", json_object_new_int(backward_iat.max));
    json_object_object_add(json_obj, "bwd_iat_min", json_object_new_int(backward_iat.min));
    json_object_object_add(json_obj, "bwd_iat_mean", json_object_new_int(backward_iat.mean));
    json_object_object_add(json_obj, "bwd_iat_std", json_object_new_int(backward_iat.std));

    // Flags statistics
    json_object_object_add(json_obj, "fwd_psh_flags", json_object_new_int(flag_counter(flow, 0x08, FORWARD)));  // PSH (Push) 0x08 FORWARD
    json_object_object_add(json_obj, "bwd_psh_flags", json_object_new_int(flag_counter(flow, 0x08, REVERSE)));  // PSH (Push) 0x08 REVERSE
    json_object_object_add(json_obj, "fwd_urg_flags", json_object_new_int(flag_counter(flow, 0x20, FORWARD)));  // URG (Urgent) 0x20 FORWARD
    json_object_object_add(json_obj, "bwd_urg_flags", json_object_new_int(flag_counter(flow, 0x20, REVERSE)));  // URG (Urgent) 0x20 REVERSE
    json_object_object_add(json_obj, "fin_flag_cnt", json_object_new_int(flag_counter(flow, 0x01, BOTH)));  // FIN (Finish) 0x01
    json_object_object_add(json_obj, "syn_flag_cnt", json_object_new_int(flag_counter(flow, 0x02, BOTH)));  // SYN (Synchronize) 0x02
    json_object_object_add(json_obj, "rst_flag_cnt", json_object_new_int(flag_counter(flow, 0x04, BOTH)));  // RST (Reset) 0x04
    json_object_object_add(json_obj, "psh_flag_cnt", json_object_new_int(flag_counter(flow, 0x08, BOTH)));  // PSH (Push) 0x08
    json_object_object_add(json_obj, "ack_flag_cnt", json_object_new_int(flag_counter(flow, 0x10, BOTH)));  // ACK (Acknowledge) 0x10
    json_object_object_add(json_obj, "urg_flag_cnt", json_object_new_int(flag_counter(flow, 0x20, BOTH)));  // URG (Urgent) 0x20
    json_object_object_add(json_obj, "ece_flag_cnt", json_object_new_int(flag_counter(flow, 0x40, BOTH)));  // ECE (ECN Echo) 0x40 
    

    // Response Time 12
    json_object_object_add(json_obj, "down_up_ratio", json_object_new_int(get_down_up_ratio(flow)));
    json_object_object_add(json_obj, "pkt_size_avg", json_object_new_int(get_avg(flow, BOTH)));
    json_object_object_add(json_obj, "init_fwd_win_byts", json_object_new_int(flow->init_window_size[FORWARD - 1]));
    json_object_object_add(json_obj, "init_bwd_win_byts", json_object_new_int(flow->init_window_size[REVERSE - 1]));
    json_object_object_add(json_obj, "active_max", json_object_new_int(active_stat.max));
    json_object_object_add(json_obj, "active_min", json_object_new_int(active_stat.min));
    json_object_object_add(json_obj, "active_mean", json_object_new_int(active_stat.mean));
    json_object_object_add(json_obj, "active_std", json_object_new_int(active_stat.std));
    json_object_object_add(json_obj, "idle_max", json_object_new_int(idle_stat.max));
    json_object_object_add(json_obj, "idle_min", json_object_new_int(idle_stat.min));
    json_object_object_add(json_obj, "idle_mean", json_object_new_int(idle_stat.mean));
    json_object_object_add(json_obj, "idle_std", json_object_new_int(idle_stat.std));  

    json_object_object_add(json_obj, "fwd_byts_b_avg", json_object_new_int(get_bytes_per_bulk(flow, FORWARD)));
    json_object_object_add(json_obj, "fwd_pkts_b_avg", json_object_new_int(get_packets_per_bulk(flow, FORWARD)));
    json_object_object_add(json_obj, "bwd_byts_b_avg", json_object_new_int(get_bytes_per_bulk(flow, REVERSE)));
    json_object_object_add(json_obj, "bwd_pkts_b_avg", json_object_new_int(get_packets_per_bulk(flow, REVERSE)));
    json_object_object_add(json_obj, "fwd_blk_rate_avg", json_object_new_int(get_bulk_rate(flow, FORWARD)));
    json_object_object_add(json_obj, "bwd_blk_rate_avg", json_object_new_int(get_bulk_rate(flow, REVERSE))); 

    // Duplicated features
    json_object_object_add(json_obj, "fwd_seg_size_avg", json_object_new_int(get_mean_len(flow, FORWARD)));
    json_object_object_add(json_obj, "bwd_seg_size_avg", json_object_new_int(get_mean_len(flow, REVERSE)));
    json_object_object_add(json_obj, "cwe_flag_count", json_object_new_int(flag_counter(flow, 0x20, FORWARD)));
    json_object_object_add(json_obj, "subflow_fwd_pkts", json_object_new_int(get_total(flow, FORWARD)));
    json_object_object_add(json_obj, "subflow_bwd_pkts", json_object_new_int(get_total(flow, REVERSE)));
    json_object_object_add(json_obj, "subflow_fwd_byts", json_object_new_int(get_total_len(flow, FORWARD)));
    json_object_object_add(json_obj, "subflow_bwd_byts", json_object_new_int(get_total_len(flow, REVERSE)));   

    // Convert JSON object to string and print it
    const char *json_str = json_object_to_json_string(json_obj);
    printf("Generated JSON:\n%s\n", json_str);
    send_json_to_clickhouse(json_str);
    // Free the JSON object
    send_json_to_python(json_str);

    json_object_put(json_obj);
}

// Add packet to flow and update bulk data
void add_packet_to_flow(Flow *flow, Packet *packet, PacketDirection direction) {
    if (flow->packets.pairs == NULL) {
        initializePackets(&flow->packets, 10);
    }
    // printf("Packet time111111111: %f \n",packet->time);
    appendPair(&flow->packets, packet, direction);
    update_flow_bulk(flow, packet, direction);
    update_subflow(flow, packet);

    // printf("\n%f\n", packet->time);

    if(flow->flow_interarrival_time.count == 0){
        initializeFlowInterarrivalTime(&flow->flow_interarrival_time);
    }
    if (flow->start_timestamp != 0) {
        // printf("Jonvor qoshildi!");
        double newTime = 1e6 * (packet->time - flow->latest_timestamp);
        appendFlowInterarrivalTime(&flow->flow_interarrival_time, newTime);
    }
    
    // Update latest timestamp
    flow->latest_timestamp = (packet->time > flow->latest_timestamp) ? packet->time : flow->latest_timestamp;
    if (packet->protocol == IPPROTO_TCP) {  // Assuming 6 represents TCP protocol
        if (direction == FORWARD && flow->init_window_size[FORWARD - 1] == 0) {
            flow->init_window_size[FORWARD - 1] = packet->window;
        } else if (direction == REVERSE) {
            flow->init_window_size[REVERSE - 1] = packet->window;
        }
    }

    // First packet of the flow
    if (flow->start_timestamp == 0) {
        flow->start_timestamp = packet->time;
        flow->protocol = packet->protocol;
    }
}

// Update subflow logic
void update_subflow(Flow *flow, Packet *packet) {
    double last_timestamp = (flow->latest_timestamp != 0) ? flow->latest_timestamp : packet->time;

    // Check if the time difference is greater than the timeout
    if ((packet->time - last_timestamp) > CLUMP_TIMEOUT) {
        // Update active and idle times
        update_active_idle(flow, packet->time - last_timestamp);
    }

    // Update the latest timestamp
    // flow->latest_timestamp = packet->time;
}

// Update active and idle times for the flow
void update_active_idle(Flow *flow, double current_time) {
    if ((current_time - flow->last_active) > ACTIVE_TIMEOUT) {
        double duration = fabs(flow->last_active - flow->start_active);  // Calculate active duration
        // printf("LAst and start %f\t%f", flow->last_active, flow->start_active);
        if(flow->active.count == 0){
            initializeActive(&flow->active, 5); 
        }
        if(flow->idle.count == 0){
            initializeIdle(&flow->idle, 5); 
        }
        // printf("Durationnnnnnnnn: %f\n", duration);
        if (duration > 0) {
            appendActive(&flow->active, 1e6 * duration);
            // printf("Activeee: %f", duration);
        }

        // Store idle time
        appendIdle(&flow->idle, 1e6 * (current_time - flow->last_active));

        // Reset active times
        flow->start_active = current_time;
        flow->last_active = current_time;
    } else {
        // Update last active time
        flow->last_active = current_time;
    }
}

// Update flow bulk logic
void update_flow_bulk(Flow *flow, Packet *packet, PacketDirection direction) {
    int payload_size = get_payload(packet);  // Assuming get_payload is defined elsewhere
    if (payload_size == 0) return;

    if (direction == FORWARD) {
        if (flow->backward_bulk_last_timestamp > flow->forward_bulk_start_tmp) {
            flow->forward_bulk_start_tmp = 0;
        }
        if (flow->forward_bulk_start_tmp == 0) {
            flow->forward_bulk_start_tmp = packet->time;
            flow->forward_bulk_last_timestamp = packet->time;
            flow->forward_bulk_count_tmp = 1;
            flow->forward_bulk_size_tmp = payload_size;
        } else {
            if (packet->time - flow->forward_bulk_last_timestamp > CLUMP_TIMEOUT) {
                flow->forward_bulk_start_tmp = packet->time;
                flow->forward_bulk_last_timestamp = packet->time;
                flow->forward_bulk_count_tmp = 1;
                flow->forward_bulk_size_tmp = payload_size;
            } else {
                flow->forward_bulk_count_tmp += 1;
                flow->forward_bulk_size_tmp += payload_size;
                if (flow->forward_bulk_count_tmp == BULK_BOUND) {
                    flow->forward_bulk_count += 1;
                    flow->forward_bulk_packet_count += flow->forward_bulk_count_tmp;
                    flow->forward_bulk_size += flow->forward_bulk_size_tmp;
                    flow->forward_bulk_duration += (packet->time - flow->forward_bulk_start_tmp);
                } else if (flow->forward_bulk_count_tmp > BULK_BOUND) {
                    flow->forward_bulk_packet_count += 1;
                    flow->forward_bulk_size += payload_size;
                    flow->forward_bulk_duration += (packet->time - flow->forward_bulk_last_timestamp);
                }
                flow->forward_bulk_last_timestamp = packet->time;
            }
        }
    } else {  // BACKWARD direction
        if (flow->forward_bulk_last_timestamp > flow->backward_bulk_start_tmp) {
            flow->backward_bulk_start_tmp = 0;
        }
        if (flow->backward_bulk_start_tmp == 0) {
            flow->backward_bulk_start_tmp = packet->time;
            flow->backward_bulk_last_timestamp = packet->time;
            flow->backward_bulk_count_tmp = 1;
            flow->backward_bulk_size_tmp = payload_size;
        } else {
            if (packet->time - flow->backward_bulk_last_timestamp > CLUMP_TIMEOUT) {
                flow->backward_bulk_start_tmp = packet->time;
                flow->backward_bulk_last_timestamp = packet->time;
                flow->backward_bulk_count_tmp = 1;
                flow->backward_bulk_size_tmp = payload_size;
            } else {
                flow->backward_bulk_count_tmp += 1;
                flow->backward_bulk_size_tmp += payload_size;
                if (flow->backward_bulk_count_tmp == BULK_BOUND) {
                    flow->backward_bulk_count += 1;
                    flow->backward_bulk_packet_count += flow->backward_bulk_count_tmp;
                    flow->backward_bulk_size += flow->backward_bulk_size_tmp;
                    flow->backward_bulk_duration += (packet->time - flow->backward_bulk_start_tmp);
                }
                if (flow->backward_bulk_count_tmp > BULK_BOUND) {
                    flow->backward_bulk_packet_count += 1;
                    flow->backward_bulk_size += payload_size;
                    flow->backward_bulk_duration += (packet->time - flow->backward_bulk_last_timestamp);
                }
                flow->backward_bulk_last_timestamp = packet->time;
            }
        }
    }
}

// Calculate flow duration
double get_flow_duration(Flow *flow) {
    return flow->latest_timestamp - flow->start_timestamp;
}
