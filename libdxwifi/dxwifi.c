/**
 * DxWifi project library implementations
 */

#include <string.h>
#include <stdbool.h>

#include <unistd.h>
#include <endian.h>

#include <libdxwifi/dxwifi.h>
#include <libdxwifi/details/utils.h>
#include <libdxwifi/details/logging.h>


static void init_dxwifi_tx_frame(dxwifi_tx_frame* frame, size_t block_size) {
    debug_assert(frame);

    frame->__frame      = (uint8_t*) calloc(1, DXWIFI_TX_HEADER_SIZE + block_size + IEEE80211_FCS_SIZE);

    frame->radiotap_hdr = (dxwifi_tx_radiotap_hdr*) frame->__frame;
    frame->mac_hdr      = (ieee80211_hdr*) (frame->__frame + sizeof(dxwifi_tx_radiotap_hdr));
    frame->payload      = frame->__frame + DXWIFI_TX_HEADER_SIZE;
}


static void teardown_dxwifi_frame(dxwifi_tx_frame* frame) {
    debug_assert(frame);

    free(frame->__frame);
    frame->__frame      = NULL;
    frame->radiotap_hdr = NULL;
    frame->mac_hdr      = NULL;
    frame->payload      = NULL;
}


static void construct_radiotap_header(dxwifi_tx_radiotap_hdr* radiotap_hdr, uint8_t flags, uint8_t rate, uint16_t tx_flags) {
    debug_assert(radiotap_hdr);

    radiotap_hdr->hdr.it_version    = IEEE80211_RADIOTAP_MAJOR_VERSION;
    radiotap_hdr->hdr.it_len        = htole16(sizeof(dxwifi_tx_radiotap_hdr));
    radiotap_hdr->hdr.it_present    = htole32(DXWIFI_TX_RADIOTAP_PRESENCE_BIT_FIELD);

    radiotap_hdr->flags     = flags;

    // Radiotap units are 500Kbps. Multiply by 2 to convert to Mbps
    radiotap_hdr->rate      = rate * 2; 

    radiotap_hdr->tx_flags  = tx_flags;
}


static void construct_ieee80211_header(ieee80211_hdr* mac_hdr) {
    debug_assert(mac_hdr);

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


static void log_configuration(const dxwifi_transmitter* tx) {
    log_info(
            "DxWifi Transmitter Initialized\n"
            "\tVerbosity:     %s\n"
            "\tDevice:        %s\n"
            "\tData Rate:     %dMbps\n"
            "\tRTAP flags:    0x%x\n"
            "\tRTAP Tx flags: 0x%x",
            log_level_to_str(tx->verbosity),
            tx->device,
            tx->rtap_rate,
            tx->rtap_flags,
            tx->rtap_tx_flags
    );
}

static void log_stats(const dxwifi_tx_frame* frame, size_t bytes_read, size_t bytes_sent, int frame_count) {
    log_debug("Frame: %d - (Bytes Read, Bytes Sent) = (%ld, %ld)", frame_count, bytes_read, bytes_sent);
    log_hexdump(frame->__frame, DXWIFI_TX_HEADER_SIZE + bytes_read + IEEE80211_FCS_SIZE);
}


void init_transmitter(dxwifi_transmitter* tx) {
    debug_assert(tx);

    char err_buff[PCAP_ERRBUF_SIZE];

    tx->__handle = pcap_open_live(
                        tx->device, 
                        SNAPLEN_MAX, 
                        true, 
                        DXWIFI_PACKET_BUFFER_TIMEOUT, 
                        err_buff
                    );

    // Hard assert here because if pcap fails it's all FUBAR anyways
    assert_M(tx->__handle != NULL, err_buff);

    log_configuration(tx);

}


void close_transmitter(dxwifi_transmitter* transmitter) {
    debug_assert(transmitter && transmitter->__handle);

    pcap_close(transmitter->__handle);

    log_info("DxWifi Transmitter closed");
}


int transmit_file(dxwifi_transmitter* transmit, int fd) {
    debug_assert(transmit && transmit->__handle);

    size_t blocksize    = transmit->block_size;
    size_t nbytes       = 0;
    int status          = 0;
    int frame_count     = 0;

    dxwifi_tx_frame data_frame = { NULL, NULL, NULL, NULL };

    init_dxwifi_tx_frame(&data_frame, blocksize);

    construct_radiotap_header(data_frame.radiotap_hdr, transmit->rtap_flags, transmit->rtap_rate, transmit->rtap_tx_flags);

    construct_ieee80211_header(data_frame.mac_hdr);

    // TODO: poll fd to see if there's any data to even read, no need to block waiting on a read
    while((nbytes = read(fd, data_frame.payload, transmit->block_size)) > 0) {


        status = pcap_inject(transmit->__handle, data_frame.__frame, DXWIFI_TX_HEADER_SIZE + nbytes + IEEE80211_FCS_SIZE);

        log_stats(&data_frame, nbytes, status, frame_count);

        debug_assert_continue(status > 0, "Injection failure: %s", pcap_statustostr(status));

        ++frame_count;
    }

    teardown_dxwifi_frame(&data_frame);
    return 0;
}
