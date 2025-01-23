#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/ip.h>       // For IP header
#include <netinet/tcp.h>      // For TCP header
#include <netinet/udp.h>      // For UDP header
#include <netinet/ip_icmp.h>  // For ICMP header
#include <net/ethernet.h>     // For Ethernet header
#include <arpa/inet.h>        // For inet_ntop
#include <time.h>
#include <unistd.h>
#include "include/packet_flow_key.h"
#include "include/packet_direction.h"
#include "include/flow.h"
#include "include/packet.h"
#include "include/packet_count.h"
#include "include/flow_session.h"
#include "include/packet_time.h"


// Packet handler function called by pcap for every captured packet
void packet_handler(u_char *user_data, const struct pcap_pkthdr *pkthdr, const u_char *packet) {
    struct ether_header *eth_hdr = (struct ether_header *)packet;

    if (ntohs(eth_hdr->ether_type) == ETHERTYPE_IP) {
        // Add your logic for processing IP packets here
    
        // Cast the user data (which is session) back to FlowSession *
        FlowSession *session = (FlowSession *)user_data;
        
        Packet packett;
        // Call your  function using the session
        on_packet_received(session, &packett, pkthdr, packet);

            // Static variable to track the last print time
        static time_t last_print_time = 0;
        time_t current_time = time(NULL);
        
        return;
    }
}

int main(int argc, char **argv) {
    pcap_t *handle;

    // Initialize the FlowSession
    FlowSession *session = init_flow_session(10);

    char errbuf[PCAP_ERRBUF_SIZE];  // Error buffer
    pcap_if_t *alldevs, *dev;       // Capture device list

    // Find all available devices
    if (pcap_findalldevs(&alldevs, errbuf) == -1) {
        fprintf(stderr, "Error in pcap_findalldevs: %s\n", errbuf);
        return 2;
    }

    // Select the first device from the list
    dev = alldevs;
    if (dev == NULL) {
        fprintf(stderr, "No devices found! Exiting.\n");
        return 2;
    }

    printf("Device: %s\n", "eno2");

    // Open the selected device for packet capture
    handle = pcap_open_live("eno2", BUFSIZ, 1, 1000, errbuf);
    if (handle == NULL) {
        fprintf(stderr, "Couldn't open device %s: %s\n", "eno2", errbuf);
        return 2;
    }

    // Free the device list
    pcap_freealldevs(alldevs);

    // Start packet capture loop, passing FlowSession * as user data
    printf("Starting packet capture...\n");
    pcap_loop(handle, -1, packet_handler, (u_char *)session);  // Pass session as user data

    // Cleanup
    pcap_close(handle);
    free_flow_session(session);  // Free session when done

    return 0;
}