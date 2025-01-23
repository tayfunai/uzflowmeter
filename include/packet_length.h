#ifndef PACKET_LENGTH_H
#define PACKET_LENGTH_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "packet_direction.h"
#include "flow.h"

// Constants
#define MAX_FIFTY 50

// Function declarations

// Function to get the packet lengths based on direction
int* get_packet_length(Flow* flow, PacketDirection packet_direction, int *length_count);

// Function to get the header lengths based on direction
int* get_header_length(Flow* flow, PacketDirection packet_direction, int *length_count);

// Function to calculate the total header length based on direction
int get_total_header(Flow* flow, PacketDirection packet_direction);

// Function to get the minimum header length
int get_min_header(Flow* flow, PacketDirection packet_direction);

// Function to get the maximum packet length
int get_max(Flow* flow, PacketDirection packet_direction);

// Function to get the minimum packet length
int get_min(Flow* flow, PacketDirection packet_direction);

// Function to calculate the total packet length
int get_total_len(Flow* flow, PacketDirection packet_direction);

// Function to calculate the average packet length
int get_avg(Flow* flow, PacketDirection packet_direction);

// Function to get the first fifty packet sizes
int* first_fifty(Flow* flow, PacketDirection packet_direction);

// Function to calculate the variance of packet lengths
double get_var_len(Flow* flow, PacketDirection packet_direction);

// Function to calculate the standard deviation of packet lengths
double get_std_len(Flow* flow, PacketDirection packet_direction);

// Function to calculate the mean of packet lengths
double get_mean_len(Flow* flow, PacketDirection packet_direction);

// Function to calculate the median of packet lengths
double get_median_len(Flow* flow, PacketDirection packet_direction);

// Function to calculate the mode of packet lengths
int get_mode_len(Flow* flow, PacketDirection packet_direction);

// Function to calculate the skewness of packet lengths using the median
double get_skew_len(Flow* flow, PacketDirection packet_direction);

// Function to calculate skewness using the mode
double get_skew2_len(Flow* flow, PacketDirection packet_direction);

// Function to calculate the coefficient of variation
double get_cov_len(Flow* flow, PacketDirection packet_direction);

#endif // PACKET_LENGTH_H
