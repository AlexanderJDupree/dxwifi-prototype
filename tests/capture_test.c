#include <pcap.h>
#include <string.h>
#include <stdbool.h>

#define SNAPLEN_MAX 65535 // Maximum length for MPDU

void usage();
void print_packet(u_char* args, const struct pcap_pkthdr* header, const u_char* packet);

int main(int argc, char** argv) {

    int return_code = 0;                    
    int num_datalinks = 0;                  /* Number of supported datalinks    */
    bool enable_monitor = false;            /* Run interface in promiscious mode*/
    int* datalink_buffer = NULL;            /* Supported datalink types         */
    pcap_t* pcap_handle = NULL;             /* Session handle                   */
    const u_char *packet;                   /* Packet data                      */
    char error_buffer[PCAP_ERRBUF_SIZE];

    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    if (argc == 3 && argv[2][0] == 'y') {
        enable_monitor = true;
    }

    // On arm platforms I've had no luck with pcap_create. Using deprecated pcap_open_live instead
    pcap_handle = pcap_open_live(argv[1], BUFSIZ, enable_monitor, 10000, error_buffer);
    if (pcap_handle == NULL) {
        fprintf(stderr, "\nErr: Coulnd't open device %s: %s\n", argv[1], error_buffer);
        return 1;
    }


/*
    pcap_handle = pcap_create(argv[1], error_buffer);
    if (pcap_handle == NULL) {
        fprintf(stderr, "Err: %s device not found\n", argv[1]);
        return 1;
    }
    if(enable_monitor && !pcap_can_set_rfmon(pcap_handle)) {
        fprintf(stderr, "Err: %s cannot be enabled in monitor mode\n", argv[1]);
        pcap_close(pcap_handle);
        return 1;
    }

    pcap_set_rfmon(pcap_handle, enable_monitor);  
    pcap_set_snaplen(pcap_handle, SNAPLEN_MAX/2);
    pcap_set_timeout(pcap_handle, 10000); // 10 seconds
    pcap_set_buffer_size(pcap_handle, BUFSIZ/2);

    // Note: interface must be currently up. `ip link set wlan0 up`
    return_code = pcap_activate(pcap_handle);
    if(return_code < 0)  // All pcap error codes are less than 0
    {
        fprintf(stderr, "pcap_activate: %s\n", pcap_statustostr(return_code));
        return 1;
    }
    if( return_code > 0) // warnings issued
    {
        fprintf(stderr, "pcap_activate: %s\n", pcap_statustostr(return_code));
    }
*/

    num_datalinks = pcap_list_datalinks(pcap_handle, &datalink_buffer);
    if( num_datalinks < 0) {
        fprintf(stderr, "pcap_list_datalinks: %s\n", pcap_statustostr(num_datalinks));
        pcap_close(pcap_handle);
        return 1;
    }
    printf("\nDatalinks:");
    for(int i = 0; i < num_datalinks; ++i) {
        printf("\n\t%s - %s", 
            pcap_datalink_val_to_name(datalink_buffer[i]),
            pcap_datalink_val_to_description(datalink_buffer[i])
            );
    }
    pcap_free_datalinks(datalink_buffer);
    printf("\nCurrent Datalink type: %s\n", pcap_datalink_val_to_name(pcap_datalink(pcap_handle)));

    //pcap_dispatch(pcap_handle, 5, print_packet, packet);
    struct pcap_pkthdr header;
    header.caplen = 0;
    header.len = 0;
    packet = pcap_next(pcap_handle, &header);
    if(packet == NULL) {
        printf("\npacket header: %d", header.caplen);
        printf("\npacket header: %d", header.len);
        printf("\nNo packets grabbed?\n");
    }
    else {
        print_packet(NULL, &header, NULL);
    }

    pcap_close(pcap_handle);
    return 0;
}

void usage(const char* name) {
    printf(
        "\nUsage: "
        "\n    %s <dev>"
        "\n",
        name
    );
}

void print_packet(u_char* args, const struct pcap_pkthdr* header, const u_char* packet) {
    printf(
        "\nPACKET FOUND:"
        "\n\tCapture Length: %d"
        "\n\tPacket Length: %d\n",
        header->caplen,
        header->len
           );
}