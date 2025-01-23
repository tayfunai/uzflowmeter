#ifndef PACKET_H
#define PACKET_H

#include <pcap.h>
#include <stdint.h>
#include <netinet/ip.h>     // For IP header
#include <netinet/tcp.h>    // For TCP header
#include <netinet/udp.h>    // For UDP header
#include <netinet/ether.h>  // For Ethernet header
#include <netinet/ip_icmp.h>  // For ICMP header
#include <arpa/inet.h>      // For inet_ntoa

// Define the Packet structure to store headers, packet time, payload size, and header size
typedef struct {
    char src_ip[INET_ADDRSTRLEN];
    char dest_ip[INET_ADDRSTRLEN];
    uint16_t src_port;
    uint16_t dest_port;
    char src_mac[18];
    char dest_mac[18];
    uint8_t protocol;     // Protocol number: 6 for TCP, 17 for UDP, 1 for ICMP, etc.
    uint8_t icmp_type;    // ICMP type (if applicable)
    uint8_t icmp_code;    // ICMP code (if applicable)
    double time;          // Timestamp of the packet
    uint16_t window;      // TCP window size (only applicable for TCP)
    int payload_size;     // Payload size in bytes
    uint8_t flags;        // TCP flags: SYN, ACK, FIN, etc. (only applicable for TCP)
    int header_size;      // Total header size (Ethernet + IP + transport layer)
    uint8_t ihl;          // Internet Header Length (IHL) in 32-bit words
} Packet;

// Function to extract headers, time, payload size, and header size from the packet
void parse_packet(const struct pcap_pkthdr *pkthdr, const u_char *packet, Packet *pkt);

#endif // PACKET_H
