#include <stdio.h>
#include <math.h>  // For sqrt(), pow()
#include <float.h> // For DBL_MAX and DBL_MIN
#include "include/utils.h"  // Include the header file

// Function to calculate statistics for a generic array
Statistics get_statistics(double *array, size_t count) {
    Statistics stats = {0};  // Initialize all values to 0

    if (count > 1) {
        double sum = 0;
        double sum_sq_diff = 0;
        stats.max = array[0];
        stats.min = array[0];

        // Calculate total, min, and max
        for (size_t i = 0; i < count; i++) {
            sum += array[i];
            if (array[i] > stats.max) stats.max = array[i];
            if (array[i] < stats.min) stats.min = array[i];
        }

        stats.total = sum;
        stats.mean = sum / count;

        // Calculate variance (for standard deviation)
        for (size_t i = 0; i < count; i++) {
            sum_sq_diff += pow(array[i] - stats.mean, 2);
        }

        stats.std = sqrt(sum_sq_diff / count);  // Standard deviation
    } else {
        // If count is <= 1, set all statistics to 0
        stats.total = 0;
        stats.max = 0;
        stats.min = 0;
        stats.mean = 0;
        stats.std = 0;
    }

    return stats;
}

// Wrapper function to get statistics for Active structure
Statistics get_active_statistics(Active *active) {
    return get_statistics(active->active, active->count);
}

// Wrapper function to get statistics for Idle structure
Statistics get_idle_statistics(Idle *idle) {
    return get_statistics(idle->idle, idle->count);
}

// Wrapper function to get statistics for FlowInterarrivalTime structure
Statistics get_flow_interarrival_statistics(FlowInterarrivalTime *flow_iat) {
    return get_statistics(flow_iat->times, flow_iat->count);
}

// // Wrapper function to get statistics for FlowInterarrivalTime structure
// Statistics get_flow_packet_iat_statistics(FlowInterarrivalTime *flow_iat) {
//     return get_statistics(flow_iat->times, flow_iat->count);
// }