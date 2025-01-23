#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <float.h>
#include "include/packet_time.h"
#include <time.h>   // For time-related functions

#define MAX_PACKETS 1000

// Global variables
int count = 0;
double *packet_times = NULL;  // Globally initialized to NULL


double* _get_packet_times(Flow *flow) {
    // Allocate memory for packet_times based on the number of packets in the flow
    packet_times = (double *)malloc(flow->packets.count * sizeof(double));
    if (packet_times == NULL) {
        printf("Memory allocation failed!\n");
        exit(1);
    }

    // Get the time of the first packet
    Pair pair = flow->packets.pairs[0];
    double first_packet_time = pair.packet->time;

    // Calculate the relative time for each packet
    for (int i = 0; i < flow->packets.count; i++) {
        Pair pair = flow->packets.pairs[i];
        packet_times[i] = pair.packet->time - first_packet_time;
    }

    return packet_times;
}


double* get_packet_iat(Flow *flow, PacketDirection packet_direction, int *iat_size) {
    // Allocate space for an array of filtered packets based on direction
    Pair *filtered_packets = (Pair *)malloc(flow->packets.count * sizeof(Pair));
    int filtered_count = 0;
    // Filter packets based on the provided packet direction (-1 for no filtering)
    for (int i = 0; i < flow->packets.count; i++) {
        Pair pair = flow->packets.pairs[i];
        // printf("vaqtdsfakuihasoiguhoidfs: %f", pair.packet->time);

        if (pair.direction == packet_direction) {
            filtered_packets[filtered_count++] = pair;  // Copy packet to filtered array
            // printf("Timeeeeeeeeeeee: %f\n", pair.packet->time);
        }
    }

    // If there are fewer than 2 packets, return NULL since IAT cannot be calculated
    if (filtered_count < 2) {
        free(filtered_packets);
        *iat_size = 0;  // No inter-arrival times
        return NULL;
    }

    // Set the size of the packet_iat array (number of IATs is one less than the number of filtered packets)
    *iat_size = filtered_count - 1;

    // Allocate space for packet IATs (one less than the number of filtered packets)
    double *packet_iat = (double *)malloc((*iat_size) * sizeof(double));
    
    // Calculate the inter-arrival times (IAT)
    for (int i = 1; i < filtered_count; i++) {
        Pair pair = filtered_packets[i];
        packet_iat[i - 1] = 1e6 * (filtered_packets[i].packet->time - filtered_packets[i-1].packet->time);
    }


    // Free the filtered packets array
    free(filtered_packets);

    return packet_iat;
}


double* relative_time_list(Flow *flow) {
    // Allocate memory for the relative_time_list
    double* relative_time_list = (double*)malloc(50 * sizeof(double));
    int limit = flow->packets.count < 50 ? flow->packets.count : 50;

    for (int index = 0; index < limit; index++) {
        if (index == 0) {
            relative_time_list[index] = 0;  // First element is 0
        } else if (index < flow->packets.count) {
            relative_time_list[index] = packet_times[index] - packet_times[index - 1];  // Difference between consecutive packet times
        } else if (index < 50) {
            relative_time_list[index] = 0;  // Append 0 if index is less than 50 and exceeds packet_count
        }
    }

    return relative_time_list;
}

// Function to get the timestamp in human-readable format

char* get_time_stamp(Flow *flow) {
    // Get the timestamp from the first packet (similar to self.flow.packets[0][0].time in Python)
    Pair pair = flow->packets.pairs[0];
    time_t raw_time = pair.packet->time;
    // Convert the time to local time (human-readable)
    struct tm *time_info = localtime(&raw_time);   // Use 'struct tm' instead of 'tm'
    
    // Allocate memory for the formatted date-time string
    static char date_time[20];  // Format: "YYYY-MM-DD HH:MM:SS" (19 characters + null terminator)
    
    // Format the time into the date_time string
    strftime(date_time, sizeof(date_time), "%Y-%m-%d %H:%M:%S", time_info);
    
    // Return the formatted string
    return date_time;
}

double get_duration(Flow *flow) {
    // printf("Meni kechirrr\n");
    double* packet_times = _get_packet_times(flow);  // Retrieve packet times
    // printf("Jonovrrrrrrrrrrrrrrr\n");
    // for(int i=0; i < flow->packets.count; i++){
    //     printf("Packet times: %f\n", packet_times[i]);
    // }
    if (packet_times == NULL || flow->packets.count <= 1) {
        free(packet_times);
        // If there are no packets or only one packet, return 0 as duration
        return 0.0;
    }
    // return 5;
    // // Find the minimum and maximum packet times
    double max_time = packet_times[0];
    double min_time = packet_times[0];

    for (int i = 1; i < flow->packets.count; i++) {
        if (packet_times[i] > max_time) {
            max_time = packet_times[i];
        }
        if (packet_times[i] < min_time) {
            min_time = packet_times[i];
        }
    }
    // if(flow->packets.count == 2){
    //     printf("First: %f \t Second: %f\n",packet_times[0], packet_times[1]);
    // }
    // Return the duration (max - min)
    free(packet_times);
    return (max_time-min_time);
    // return flow->latest_timestamp - flow->start_timestamp;
}


double get_var(Flow *flow) {
    double* packet_times = _get_packet_times(flow);  // Retrieve packet times
    int packet_count = flow->packets.count;  // Assuming packet_count is stored in the flow

    if (packet_count == 0) {
        return 0.0;  // Return 0 variance if there are no packets
    }

    // Step 1: Calculate the mean of the packet times
    double sum = 0.0;
    for (int i = 0; i < packet_count; i++) {
        sum += packet_times[i];
    }
    double mean = sum / packet_count;

    // Step 2: Calculate the variance
    double variance = 0.0;
    for (int i = 0; i < packet_count; i++) {
        variance += pow(packet_times[i] - mean, 2);
    }

    // Divide by packet_count to get the final variance
    variance /= packet_count;
    free(packet_times);
    return variance;
}

double get_std(Flow *flow) {
    // Standard deviation is the square root of the variance
    return sqrt(get_var(flow));  // Call get_var() to get the variance and compute sqrt
}

double get_mean(Flow *flow) {
    double* packet_times = _get_packet_times(flow);  // Retrieve packet times
    int packet_count = flow->packets.count;  // Assuming packet_count is stored in the flow

    if (packet_times == NULL || packet_count == 0) {
        free(packet_times);
        return 0.0;  // Return 0 if there are no packets
    }

    double sum = 0.0;
    for (int i = 0; i < packet_count; i++) {
        sum += packet_times[i];
    }
    free(packet_times);
    // Return the mean (sum / packet_count)
    return sum / packet_count;
}

// Comparator function for qsort (sorts doubles in ascending order)
int compare_doubles(const void *a, const void *b) {
    double arg1 = *(const double*)a;
    double arg2 = *(const double*)b;
    
    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}

double get_median(Flow *flow) {
    double *packet_times = _get_packet_times(flow);  // Retrieve packet times
    int packet_count = flow->packets.count;  // Assuming packet_count is stored in the flow

    if (packet_times == NULL || packet_count == 0) {
        return 0.0;  // Return 0 if there are no packets
    }

    // Sort the packet_times array in ascending order using qsort
    qsort(packet_times, packet_count, sizeof(double), compare_doubles);

    double median;
    // If the number of packets is odd, get the middle element
    if (packet_count % 2 == 1) {
        median = packet_times[packet_count / 2];
    } else {
        // If the number of packets is even, average the two middle elements
        int mid_index = packet_count / 2;
        median = (packet_times[mid_index - 1] + packet_times[mid_index]) / 2.0;
    }

    free(packet_times);  // Free packet_times after use
    return median;  // Return the calculated median
}


double get_mode(Flow *flow) {
    double *packet_times = _get_packet_times(flow);  // Retrieve packet times
    int packet_count = flow->packets.count;  // Assuming packet_count is stored in the flow

    if (packet_times == NULL || packet_count == 0) {
        return -1.0;  // Return -1 if there are no packets (or no mode)
    }

    // Sort the packet times first to make counting easier
    qsort(packet_times, packet_count, sizeof(double), compare_doubles);

    // Variables to track the mode
    double mode = packet_times[0];
    int max_count = 1;
    int current_count = 1;

    // Loop through the sorted packet times and find the most frequent value
    for (int i = 1; i < packet_count; i++) {
        if (packet_times[i] == packet_times[i - 1]) {
            current_count++;  // Same value as before, increment the count
        } else {
            if (current_count > max_count) {
                max_count = current_count;  // Update the max count
                mode = packet_times[i - 1];  // Update the mode
            }
            current_count = 1;  // Reset count for the new value
        }
    }

    // Final check at the end of the loop
    if (current_count > max_count) {
        mode = packet_times[packet_count - 1];  // Last element is the mode
    }
    free(packet_times);
    return mode;
}



// Function to calculate the skew of packet times using the median
double get_skew(Flow *flow) {
    double mean = get_mean(flow);       // Calculate the mean
    double median = get_median(flow);   // Calculate the median
    double std = get_std(flow);         // Calculate the standard deviation
    double skew = -10;                  // Default skew value when std is 0

    // Calculate the skew if std is not zero
    if (std != 0) {
        double dif = 3 * (mean - median);  // Calculate 3 * (mean - median)
        skew = dif / std;                  // Calculate skew
    }

    return skew;
}


double get_skew2(Flow *flow) {
    double mean = get_mean(flow);       // Calculate the mean
    double mode = get_mode(flow);       // Calculate the mode
    double std = get_std(flow);         // Calculate the standard deviation
    double skew2 = -10;                 // Default skew value when std is 0

    // Calculate the skew2 if std is not zero
    if (std != 0) {
        double dif = mean - mode;       // Calculate mean - mode
        skew2 = dif / std;              // Calculate skew2
    }

    return skew2;
}

double get_cov(Flow* flow) {
    double mean = get_mean(flow);   // Calculate the mean of packet times
    double cov = -1.0;              // Initialize cov to -1.0 (default)

    // If the mean is not zero, calculate cov as (std / mean)
    if (mean != 0) {
        cov = get_std(flow) / mean;
    }

    return cov;
}


