/**
 * DxWifi project library implementations
 */

#include <string.h>
#include <stdbool.h>

#include <unistd.h>
#include <endian.h>

#include <dxwifi/utils.h>
#include <dxwifi/dxwifi.h>

void init_dxwifi_tx_frame(dxwifi_tx_frame* frame, size_t block_size) {

    frame->__frame      = (uint8_t*) calloc(1, DXWIFI_TX_HEADER_SIZE + block_size + IEEE80211_FCS_SIZE);

    frame->radiotap_hdr = (dxwifi_tx_radiotap_hdr*) frame->__frame;
    frame->mac_hdr      = (ieee80211_hdr*) (frame->__frame + sizeof(dxwifi_tx_radiotap_hdr));
    frame->payload      = frame->__frame + DXWIFI_TX_HEADER_SIZE;
}


void teardown_dxwifi_frame(dxwifi_tx_frame* frame) {
    free(frame->__frame);
    frame->__frame      = NULL;
    frame->radiotap_hdr = NULL;
    frame->mac_hdr      = NULL;
    frame->payload      = NULL;
}


void construct_dxwifi_header(dxwifi_tx_frame* frame) {
    assert_not_null(frame);

    construct_radiotap_header(frame->radiotap_hdr, DXWIFI_DFLT_RADIOTAP_FLAGS, DXWIFI_BITRATE, DXWIFI_DFLT_RADIOTAP_TX_FLAGS);
    construct_ieee80211_header(frame->mac_hdr);

}


void construct_radiotap_header(dxwifi_tx_radiotap_hdr* radiotap_hdr, uint8_t flags, uint8_t rate, uint16_t tx_flags) {

    radiotap_hdr->hdr.it_version    = IEEE80211_RADIOTAP_MAJOR_VERSION;
    radiotap_hdr->hdr.it_len        = htole16(sizeof(dxwifi_tx_radiotap_hdr));
    radiotap_hdr->hdr.it_present    = htole32(DXWIFI_RADIOTAP_PRESENCE_BIT_FIELD);

    radiotap_hdr->flags     = flags;
    radiotap_hdr->rate      = rate * 2;
    radiotap_hdr->tx_flags  = tx_flags;
}


void construct_ieee80211_header(ieee80211_hdr* mac_hdr) {
    #define WLAN_FC_TYPE_DATA	    2
    #define WLAN_FC_SUBTYPE_DATA    0

    // TODO THIS IS ALL FUBAR AND NOT HOW IT SHOULD BE DONE!!!!!

    const uint8_t mac[6] = { 0x05, 0x03, 0x05, 0x03, 0x05, 0x03 };

    uint8_t frame_control[2];
    frame_control[0] = ((WLAN_FC_TYPE_DATA << 2) | (WLAN_FC_SUBTYPE_DATA << 4));
    frame_control[1] = 0x02;

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


void init_transmitter(dxwifi_transmitter* transmitter, const char* dev_name) {
    char err_buff[PCAP_ERRBUF_SIZE];

    transmitter->handle = pcap_open_live(
                            dev_name, 
                            SNAPLEN_MAX, 
                            true, 
                            DXWIFI_DFLT_PACKET_BUFFER_TIMEOUT, 
                            err_buff
                            );

    // Hard assert here because if pcap fails it's all FUBAR anyways
    assert_M(transmitter->handle != NULL, err_buff);
}

void close_transmitter(dxwifi_transmitter* transmitter) {
    debug_assert_not_null(transmitter);
    debug_assert_not_null(transmitter->handle);
    pcap_close(transmitter->handle);
}


int transmit_file(dxwifi_transmitter* transmit, int fd, size_t blocksize) {
    debug_assert_not_null(transmit);

    // TODO All these print statements are going bye-bye

    size_t nbytes   = 0;
    int status      = 0;
    int frame_count = 0;

    dxwifi_tx_frame data_frame = { NULL, NULL, NULL, NULL };

    init_dxwifi_tx_frame(&data_frame, blocksize);

    construct_dxwifi_header(&data_frame);

    printf("\nDXWifi Header size: %ld\n", DXWIFI_TX_HEADER_SIZE);

    // TODO: poll fd to see if there's any data to even read, no need to block waiting on a read
    while((nbytes = read(fd, data_frame.payload, blocksize)) > 0) {

        printf("nbytes: %ld\n", nbytes);

        // TODO Prepare data, FEC Encode etc. 
        // TODO we may need to zero-extend the last block of data read if it's too small

        memset(data_frame.payload, 0xFF, nbytes); // Debuggging

        hexdump(data_frame.__frame, DXWIFI_TX_HEADER_SIZE + blocksize + IEEE80211_FCS_SIZE);
        status = pcap_inject(transmit->handle, data_frame.__frame, DXWIFI_TX_HEADER_SIZE + nbytes + IEEE80211_FCS_SIZE);

        printf("Bytes read: %ld\n", nbytes);

        printf("Bytes sent: %d\n", status);

        debug_assert_continue(status > 0, "Injection failure: %s", pcap_statustostr(status));

        ++frame_count;
    }

    teardown_dxwifi_frame(&data_frame);
        
    printf("frames: %d\n", frame_count);
    return 0;
}
