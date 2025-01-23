#ifndef UTILS_H
#define UTILS_H
#include "flow.h"

#include <stddef.h> // For size_t

// Structure to hold the statistics results
typedef struct {
    double total;
    double max;
    double min;
    double mean;
    double std;
} Statistics;


// Function to calculate statistics for a generic array
Statistics get_statistics(double *array, size_t count);

// Wrapper functions for specific structures
Statistics get_active_statistics(Active *active);
Statistics get_idle_statistics(Idle *idle);
Statistics get_flow_interarrival_statistics(FlowInterarrivalTime *flow_iat);

#endif // UTILS_H
