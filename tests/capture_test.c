/**
 *  basic hello world exercise for pcap
 */

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
    pcap_handle = pcap_open_live(argv[1], SNAPLEN_MAX, enable_monitor, 10000, error_buffer);
    if (pcap_handle == NULL) {
        fprintf(stderr, "\nErr: Failed to open device %s: %s\n", argv[1], error_buffer);
        return 1;
    }

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

    pcap_dispatch(pcap_handle, 5, print_packet, packet);

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