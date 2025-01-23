#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#include "include/packet_length.h"
// #include "packet_direction.h"

#define MAX_FIFTY 50


static int mean_count = 0;
static double grand_total = 0;


#include <stdlib.h>

int* get_packet_length(Flow* flow, PacketDirection packet_direction, int *length_count) {
    // Allocate memory dynamically to store the packet lengths
    int *packet_lengths = (int*) malloc(flow->packets.count * sizeof(int));
    if (packet_lengths == NULL) {
        // Handle memory allocation failure
        perror("Unable to allocate memory for packet lengths");
        return NULL;
    }

    int count = 0;
    for (int i = 0; i < flow->packets.count; i++) {
        Pair pair = flow->packets.pairs[i];
        // Check if packet direction matches (or if packet_direction is -1, indicating no filtering)
        if (packet_direction == BOTH){
            packet_lengths[count] = pair.packet->payload_size + pair.packet->header_size;  // Store the length of the packet
            count++;
        }else if (pair.direction == packet_direction){
            packet_lengths[count] = pair.packet->payload_size + pair.packet->header_size;  // Store the length of the packet
            count++;
        }
        
    }

    *length_count = count;  // Store the count of valid packets in the out parameter

    // Optionally, you could realloc the array to the actual size to avoid unused memory
    packet_lengths = realloc(packet_lengths, count * sizeof(int));

    return packet_lengths;  // Return the dynamically allocated array of packet lengths
}



// Function to calculate packet header lengths based on direction
int* get_header_length(Flow* flow, PacketDirection packet_direction, int *length_count) {
    // Allocate memory to store the header lengths
    int* header_lengths = (int*)malloc(flow->packets.count * sizeof(int));
    if (!header_lengths) {
        printf("Memory allocation failed!\n");
        exit(1);
    }

    int count = 0;
    for (int i = 0; i < flow->packets.count; i++) {
        Pair pair = flow->packets.pairs[i];
        // Check if packet direction matches (or if packet_direction is -1, indicating no filtering)
        if (pair.direction == packet_direction) {
            header_lengths[count] = pair.packet->ihl*4;  // Store the length of the packet
            count++;
        }
    }

    *length_count = count;  // Store the count of valid packets in the out parameter

    // Optionally, you could realloc the array to the actual size to avoid unused memory
    header_lengths = realloc(header_lengths, count * sizeof(int));
    return header_lengths;  // Return the array of header lengths
}

// Function to calculate the total (sum) of header lengths
int get_total_header(Flow* flow, PacketDirection packet_direction) {
    int header_length_count = 0;
    int* header_lengths = get_header_length(flow, packet_direction, &header_length_count);  // Get header lengths
    
    int total_header_length = 0;
    
    // Sum up the header lengths
    for (int i = 0; i < header_length_count; i++) {
        total_header_length += header_lengths[i];
    }

    // Free the dynamically allocated memory for header_lengths
    free(header_lengths);

    return total_header_length;  // Return the total sum of header lengths
}

int get_min_header(Flow* flow, PacketDirection packet_direction) {
    int header_length_count = 0;  // To store the number of valid packet lengths
    int* header_lengths = get_header_length(flow, packet_direction, &header_length_count);  // Get header lengths
    
    if (header_length_count == 0) {
        free(header_lengths);
        return -1;  // Return -1 if no valid header lengths are found
    }

    // Initialize min_header_length to the first value in the array
    int min_header_length = header_lengths[0];

    // Loop through the rest of the header lengths to find the minimum
    for (int i = 1; i < header_length_count; i++) {
        if (header_lengths[i] < min_header_length) {
            min_header_length = header_lengths[i];
        }
    }

    // Free the dynamically allocated memory for header_lengths
    free(header_lengths);

    return min_header_length;  // Return the minimum header length
}


int get_max(Flow* flow, PacketDirection packet_direction) {
    int packet_length_count = 0;  // To store the number of valid packet lengths
    int* packet_lengths = get_packet_length(flow, packet_direction, &packet_length_count);  // Get packet lengths
    
    if (packet_length_count == 0) {
        free(packet_lengths);
        return 0;  // Return 0 if no valid packet lengths are found
    }

    // Initialize max_packet_length to the first value in the array
    int max_packet_length = packet_lengths[0];

    // Loop through the packet lengths to find the maximum
    for (int i = 1; i < packet_length_count; i++) {
        if (packet_lengths[i] > max_packet_length) {
            max_packet_length = packet_lengths[i];
        }
    }

    // Free the dynamically allocated memory for packet_lengths
    free(packet_lengths);

    return max_packet_length;  // Return the maximum packet length
}

int get_min(Flow* flow, PacketDirection packet_direction) {
    int packet_length_count = 0;  // To store the number of valid packet lengths
    int* packet_lengths = get_packet_length(flow, packet_direction, &packet_length_count);  // Get packet lengths

    // If no valid packet lengths are found, return 0
    if (packet_length_count == 0) {
        free(packet_lengths);  // Free the memory allocated for packet_lengths
        return 0;  // Return 0 if no valid packet lengths
    }

    // Initialize min_packet_length to the largest possible value
    int min_packet_length = packet_lengths[0];

    // Loop through the packet lengths to find the minimum
    for (int i = 0; i < packet_length_count; i++) {
        if (packet_lengths[i] < min_packet_length) {
            min_packet_length = packet_lengths[i];
        }
    }

    // Free the dynamically allocated memory for packet_lengths
    free(packet_lengths);

    return min_packet_length;  // Return the minimum packet length
}

int get_total_len(Flow* flow, PacketDirection packet_direction) {
    int total_packet_count = 0;
    int total_packet_length = 0;
    // Summing up the packet lengths based on direction
    int* packet_lengths = get_packet_length(flow, packet_direction, &total_packet_count);
    for(int i = 0; i<total_packet_count; i++){
        total_packet_length += packet_lengths[i];
    
    }
    free(packet_lengths);

    return total_packet_length;
}

int get_avg(Flow* flow, PacketDirection packet_direction) {
    int count = 0;
    int* packet_lengths = get_packet_length(flow, packet_direction, &count);  // Get packet lengths
    

    int avg = 0;
    if (count > 0) {
        avg = get_total_len(flow, packet_direction) / count;
    }

    // Free packet_lengths after use, regardless of count
    free(packet_lengths);

    return avg;  // Return the calculated average or 0 if count is 0
}

// Function to get the first 50 packet sizes
int* first_fifty(Flow* flow, PacketDirection packet_direction) {
    int count = 0;
    int* packet_lengths = get_packet_length(flow, packet_direction, &count);

    // Allocate memory for the first 50 packet lengths
    int* first_fifty_packet_lengths = (int*)malloc(count * sizeof(int));
    if (!first_fifty_packet_lengths) {
        printf("Memory allocation failed!\n");
        free(packet_lengths);  // Free the previously allocated memory
        exit(1);
    }

    // Copy the first 50 (or fewer) packet lengths
    for (int i = 0; i < count && count < MAX_FIFTY; i++) {
        first_fifty_packet_lengths[i] = packet_lengths[i];
    }

    free(packet_lengths);  // Free the packet lengths array after copying

    return first_fifty_packet_lengths;
}

double get_var_len(Flow* flow, PacketDirection packet_direction) {
    int count = 0;
    int* packet_lengths = get_packet_length(flow, packet_direction, &count);
    if (count == 0) {
        free(packet_lengths);
        return 0.0; // No packets, return 0 variance
    }

    // Calculate mean
    double sum = 0.0;
    for (int i = 0; i < count; i++) {
        sum += packet_lengths[i];
    }

    double mean = sum / count;

    // Calculate variance
    double variance = 0.0;
    for (int i = 0; i < count; i++) {
        variance += pow(packet_lengths[i] - mean, 2);
    }
    variance /= count;

    // Free the allocated memory
    free(packet_lengths);

    return variance;
}

double get_std_len(Flow* flow, PacketDirection packet_direction) {
    double variance = get_var_len(flow, packet_direction);
    return sqrt(variance);
}

// Function to calculate the mean of packet lengths
double get_mean_len(Flow* flow, PacketDirection packet_direction) {
    int count = 0;
    int* packet_lengths = get_packet_length(flow, packet_direction, &count);

    if (count == 0) {
        free(packet_lengths);
        return 0.0; // No packets, return 0 as the mean
    }

    // Calculate the sum of the packet lengths
    double sum = 0.0;
    for (int i = 0; i < count; i++) {
        sum += packet_lengths[i];
    }

    // Calculate the mean
    double mean = sum / count;

    // Free the allocated memory
    free(packet_lengths);

    return mean;
}

// double get_median_len(Flow* flow, PacketDirection packet_direction) {
//     int count = 0;
//     int* packet_lengths = get_packet_length(flow, packet_direction, &count);

//     if (count == 0) {
//         free(packet_lengths);
//         return 0.0;  // No packets, return 0 as the median
//     }

//     // Sort the packet lengths array
//     qsort(packet_lengths, length_count, sizeof(int), compare);

//     // Calculate the median
//     double median;
//     if (length_count % 2 == 0) {
//         // Even number of elements, take the average of the two middle elements
//         median = (packet_lengths[length_count / 2 - 1] + packet_lengths[length_count / 2]) / 2.0;
//     } else {
//         // Odd number of elements, take the middle element
//         median = packet_lengths[length_count / 2];
//     }

//     // Free the allocated memory
//     free(packet_lengths);

//     return median;
// }

double get_median_len(Flow* flow, PacketDirection packet_direction) {
    int count = 0;
    int* packet_lengths = get_packet_length(flow, packet_direction, &count);

    if (count == 0) {
        free(packet_lengths);
        return 0.0;  // No packets, return 0 as the median
    }

    // Define a comparison function as a function pointer (use qsort-compatible syntax)
    int compare_ints(const void *a, const void *b) {
        return (*(int*)a - *(int*)b);
    }

    // Use qsort with the inline compare function
    qsort(packet_lengths, count, sizeof(int), compare_ints);

    // Calculate the median
    double median;
    if (count % 2 == 0) {
        // Even number of elements, take the average of the two middle elements
        median = (packet_lengths[count / 2 - 1] + packet_lengths[count / 2]) / 2.0;
    } else {
        // Odd number of elements, take the middle element
        median = packet_lengths[count / 2];
    }

    // Free the allocated memory
    free(packet_lengths);

    return median;
}




int get_mode_len(Flow* flow, PacketDirection packet_direction) {
    int count = 0;
    int* packet_lengths = get_packet_length(flow, packet_direction, &count);

    if (count == 0) {
        free(packet_lengths);
        return -1;  // No packets, return -1 as the mode
    }

    // Find the mode of the packet lengths
    int max_count = 0;
    int mode = packet_lengths[0];

    // Iterate over each packet length to find the mode
    for (int i = 0; i < count; i++) {
        int count = 0;
        for (int j = 0; j < count; j++) {
            if (packet_lengths[j] == packet_lengths[i]) {
                count++;
            }
        }
        // Update the mode if a higher count is found
        if (count > max_count) {
            max_count = count;
            mode = packet_lengths[i];
        }
    }

    // Free the allocated memory
    free(packet_lengths);

    return mode;
}


// Function to calculate the skew of packet times using the median
double get_skew_len(Flow* flow, PacketDirection packet_direction) {
    double mean = get_mean_len(flow, packet_direction);       // Calculate the mean
    double median = get_median_len(flow, packet_direction);   // Calculate the median
    double std = get_std_len(flow, packet_direction);         // Calculate the standard deviation
    double skew = -10;                  // Default skew value when std is 0

    // Calculate the skew if std is not zero
    if (std != 0) {
        double dif = 3 * (mean - median);  // Calculate 3 * (mean - median)
        skew = dif / std;                  // Calculate skew
    }

    return skew;
}


double get_skew2_len(Flow* flow, PacketDirection packet_direction) {
    double mean = get_mean_len(flow, packet_direction);       // Calculate the mean
    double mode = get_mode_len(flow, packet_direction);       // Calculate the mode
    double std = get_std_len(flow, packet_direction);         // Calculate the standard deviation
    double skew2 = -10;                 // Default skew value when std is 0

    // Calculate the skew2 if std is not zero
    if (std != 0) {
        double dif = mean - mode;       // Calculate mean - mode
        skew2 = dif / std;              // Calculate skew2
    }

    return skew2;
}

double get_cov_len(Flow* flow, PacketDirection packet_direction) {
    double mean = get_mean_len(flow, packet_direction);   // Calculate the mean of packet times
    double cov = -1.0;              // Initialize cov to -1.0 (default)

    // If the mean is not zero, calculate cov as (std / mean)
    if (mean != 0) {
        cov = get_std_len(flow, packet_direction) / mean;
    }

    return cov;
}