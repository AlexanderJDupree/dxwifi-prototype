/**
 * DxWifi project library implementations
 */

#include <string.h>
#include <stdbool.h>

#include <unistd.h>

#include <dxwifi/utils.h>
#include <dxwifi/dxwifi.h>

void init_dxwifi_frame(dxwifi_frame* frame, size_t block_size) {
    frame->__frame = (uint8_t*) calloc(1, DXWIFI_HEADER_SIZE + block_size + 4);

    frame->radiotap_hdr = (ieee80211_radiotap_hdr*) frame->__frame;  
    frame->mac_hdr      = (ieee80211_hdr*) (frame->__frame + sizeof(ieee80211_radiotap_hdr) + 4);
    frame->payload      = (frame->__frame + sizeof(ieee80211_radiotap_hdr) + sizeof(ieee80211_hdr) + 4);

    // Sanity check
}


void teardown_dxwifi_frame(dxwifi_frame* frame) {
    // Deallocate frame buffer and point members to NULL
    free(frame->__frame);
    frame->__frame      = NULL;
    frame->radiotap_hdr = NULL;
    frame->mac_hdr      = NULL;
    frame->payload      = NULL;
}


void construct_dxwifi_header(dxwifi_frame* frame) {
    assert_not_null(frame);

    construct_radiotap_header(frame->radiotap_hdr);
    construct_ieee80211_header(frame->mac_hdr);
}

void construct_ieee80211_header(ieee80211_hdr* mac_hdr) {
    #define WLAN_FC_TYPE_DATA	    2
    #define WLAN_FC_SUBTYPE_DATA    0

    const uint8_t mac[6] = { 0x05, 0x03, 0x05, 0x03, 0x05, 0x03 };

    uint8_t frame_control[2];
    frame_control[0] = ((WLAN_FC_TYPE_DATA << 2) | (WLAN_FC_SUBTYPE_DATA << 4));
    frame_control[1] = 0x01;

    memcpy(&mac_hdr->frame_control, frame_control, sizeof(uint16_t));

    mac_hdr->duration_id = 0xffff;

    memset(&mac_hdr->addr1[0], 0, 6 * sizeof(uint8_t));
    mac_hdr->addr1[0] = 0x01;
    mac_hdr->addr1[1] = 0x02;

    memset(&mac_hdr->addr3[0], 0, 6 * sizeof(uint8_t));
    mac_hdr->addr3[1] = 0x05;
    mac_hdr->addr3[3] = 0x07;


    memcpy(&mac_hdr->addr2[0], mac, 6*sizeof(uint8_t));

    mac_hdr->seq_ctrl = 0;

    #undef WLAN_FC_TYPE_DATA
    #undef WLAN_FC_SUBTYPE_DATA
}


void construct_radiotap_header(ieee80211_radiotap_hdr* radiotap_hdr) {

    // TODO programatically add fields
    const uint8_t u8aRadiotapHeader[] = {
        0x00, 0x00, 0x0C, 0x00, 0x06, 0x80, 0x00, 0x00, 0x10, 0x0c, 0x08, 0x00 
    };

    memcpy(radiotap_hdr, u8aRadiotapHeader, sizeof(u8aRadiotapHeader));

}


void init_transmitter(dxwifi_transmitter* transmitter, const char* dev_name) {
    char err_buff[PCAP_ERRBUF_SIZE];

    transmitter->handle = pcap_open_live(
                            dev_name, 
                            SNAPLEN_MAX, 
                            true, 
                            DXWIFI_DFLT_PACKET_BUFFER_TIMEOUT, 
                            err_buff
                            );
    assert_M(transmitter->handle != NULL, err_buff);
}

void close_transmitter(dxwifi_transmitter* transmitter) {
    assert_not_null(transmitter);
    assert_not_null(transmitter->handle);
    pcap_close(transmitter->handle);
}


int transmit_file(dxwifi_transmitter* transmit, int fd, size_t blocksize) {
    assert_not_null(transmit);

    size_t nbytes   = 0;
    int status      = 0;
    int frame_count = 0;
    dxwifi_frame data_frame = { NULL, NULL, NULL, NULL };

    init_dxwifi_frame(&data_frame, blocksize);

    // TODO: poll fd to see if there's any data to even read
    while((nbytes = read(fd, data_frame.payload, blocksize)) > 0) {
        printf("nbytes: %ld\n", nbytes);

        // Prepare data, FEC Encode etc. 
        construct_dxwifi_header(&data_frame);

        printf("\nDXWifi Header size: %ld\n", DXWIFI_HEADER_SIZE);
        //hexdump(data_frame.__frame, DXWIFI_HEADER_SIZE + blocksize);
        status = pcap_inject(transmit->handle, data_frame.__frame, DXWIFI_HEADER_SIZE + nbytes);
        printf("Bytes read: %ld\n", nbytes);
        printf("Bytes sent: %d\n", status);

        debug_assert_continue(status > 0, "Injection failure: %s", pcap_statustostr(status));

        ++frame_count;
    }

    teardown_dxwifi_frame(&data_frame);
        
    printf("frames: %d\n", frame_count);
    return 0;
}
