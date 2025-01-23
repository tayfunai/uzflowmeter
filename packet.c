#include "include/packet.h"
#include "include/packet_count.h"
#include <string.h> // For memset

// Function to parse the packet and populate the Packet structure (headers + time + payload size)
void parse_packet(const struct pcap_pkthdr *pkthdr, const u_char *packet, Packet *pkt) {
    memset(pkt, 0, sizeof(Packet));  // Initialize the structure

    // Parse Ethernet header
    struct ether_header *eth_hdr = (struct ether_header *)packet;
    
    // Convert MAC addresses to human-readable format
    snprintf(pkt->src_mac, sizeof(pkt->src_mac), "%02x:%02x:%02x:%02x:%02x:%02x",
             eth_hdr->ether_shost[0], eth_hdr->ether_shost[1], eth_hdr->ether_shost[2],
             eth_hdr->ether_shost[3], eth_hdr->ether_shost[4], eth_hdr->ether_shost[5]);
    snprintf(pkt->dest_mac, sizeof(pkt->dest_mac), "%02x:%02x:%02x:%02x:%02x:%02x",
             eth_hdr->ether_dhost[0], eth_hdr->ether_dhost[1], eth_hdr->ether_dhost[2],
             eth_hdr->ether_dhost[3], eth_hdr->ether_dhost[4], eth_hdr->ether_dhost[5]);

    // Check if it's an IP packet (Ethernet type must be IPv4)
    if (ntohs(eth_hdr->ether_type) != ETHERTYPE_IP) {
        return;  // Return if it's not an IP packet
    }

    // Parse IP header
    struct iphdr *ip_hdr = (struct iphdr *)(packet + sizeof(struct ether_header));

    // Store the Internet Header Length (IHL)
    pkt->ihl = ip_hdr->ihl;

    // Convert IP addresses to human-readable format
    inet_ntop(AF_INET, &(ip_hdr->saddr), pkt->src_ip, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &(ip_hdr->daddr), pkt->dest_ip, INET_ADDRSTRLEN);

    // Store protocol number (e.g., 6 for TCP, 17 for UDP, 1 for ICMP)
    pkt->protocol = ip_hdr->protocol;

    // Variables to calculate payload and header size
    int ip_header_len = ip_hdr->ihl * 4;  // IP header length in bytes
    int transport_header_len = 0;         // Will be set depending on protocol
    int total_length = ntohs(ip_hdr->tot_len);  // Total length of the IP packet
    int ethernet_header_len = sizeof(struct ether_header); // Ethernet header size

    // Check if the protocol is TCP, UDP, or ICMP
    if (ip_hdr->protocol == IPPROTO_TCP) {
        // Parse TCP header
        struct tcphdr *tcp_hdr = (struct tcphdr *)(packet + ethernet_header_len + ip_header_len);
        pkt->src_port = ntohs(tcp_hdr->source);
        pkt->dest_port = ntohs(tcp_hdr->dest);
        transport_header_len = tcp_hdr->doff * 4;  // TCP header length in bytes

        // Extract and store the TCP window size
        pkt->window = ntohs(tcp_hdr->window);

        // Extract and store the TCP flags
        pkt->flags = tcp_hdr->th_flags;  // Store the TCP flags (SYN, ACK, FIN, etc.)

    } else if (ip_hdr->protocol == IPPROTO_UDP) {
        // Parse UDP header
        struct udphdr *udp_hdr = (struct udphdr *)(packet + ethernet_header_len + ip_header_len);
        pkt->src_port = ntohs(udp_hdr->source);
        pkt->dest_port = ntohs(udp_hdr->dest);
        transport_header_len = sizeof(struct udphdr);  // UDP header has a fixed size

        // Set window size as 0 for UDP, as it doesn't use a window size
        pkt->window = 0;

    } else if (ip_hdr->protocol == IPPROTO_ICMP) {
        // Parse ICMP header
        struct icmphdr *icmp_hdr = (struct icmphdr *)(packet + ethernet_header_len + ip_header_len);
        pkt->icmp_type = icmp_hdr->type;
        pkt->icmp_code = icmp_hdr->code;
        transport_header_len = sizeof(struct icmphdr);  // ICMP header has a fixed size

        // Set window size to 0 for ICMP
        pkt->window = 0;

    } else {
        // For other protocols, set window to 0
        pkt->window = 0; 
    }

    // Calculate the payload size
    pkt->payload_size = total_length - (ip_header_len + transport_header_len);
    if (pkt->payload_size < 0) {
        pkt->payload_size = 0;  // Ensure the payload size is non-negative
    }

    // Calculate the total header size (Ethernet + IP + transport)
    pkt->header_size = ethernet_header_len + ip_header_len + transport_header_len;

    // Get the packet timestamp (seconds + microseconds)
    pkt->time = (double)pkthdr->ts.tv_sec + (double)pkthdr->ts.tv_usec / 1000000.0;
}
