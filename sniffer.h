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


// Packet handler function called by pcap for every captured packet
void packet_handler(u_char *user_data, const struct pcap_pkthdr *pkthdr, const u_char *packet) {
 
    struct ether_header *eth_hdr = (struct ether_header *)packet;

    // Only process IP packets (Ethernet type must be IPv4)
    if (ntohs(eth_hdr->ether_type) != ETHERTYPE_IP) {
        printf("Not an IP packet\n");
        return;
    }

    // Print the length of the captured packet
    printf("Captured a packet of length: %d\n", pkthdr->len);
}

int main(int argc, char **argv) {
    pcap_t *handle;
    char errbuf[PCAP_ERRBUF_SIZE];  // Error buffer
    char *dev;                      // Capture device name

    // Find a device to capture from (e.g., eth0, wlan0)
    dev = pcap_lookupdev(errbuf);
    if (dev == NULL) {
        fprintf(stderr, "Couldn't find default device: %s\n", errbuf);
        return 2;
    }
    printf("Device: %s\n", dev);

    // Open the device for packet capture
    handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
    if (handle == NULL) {
        fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
        return 2;
    }

    // Start packet capture loop (no user data needed for now, so we pass NULL)
    printf("Starting packet capture...\n");
    pcap_loop(handle, 10, packet_handler, NULL);  // Capture 10 packets

    // Cleanup
    pcap_close(handle);

    return 0;
}
