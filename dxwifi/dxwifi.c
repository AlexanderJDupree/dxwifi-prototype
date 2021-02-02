/**
 * DxWifi project library implementations
 */

#include <stdbool.h>

#include <unistd.h>

#include <dxwifi/utils.h>
#include <dxwifi/dxwifi.h>

void init_dxwifi_frame(dxwifi_frame* frame, size_t block_size) {
    frame->__frame = (uint8_t*) calloc(1, DXWIFI_HEADER_SIZE + block_size);

    frame->radiotap_hdr = frame->__frame;  
    frame->mac_hdr      = frame->radiotap_hdr + sizeof(struct ieee80211_radiotap_header);
    frame->payload      = frame->mac_hdr + sizeof(struct ieee80211_hdr);
}

void teardown_dxwifi_frame(dxwifi_frame* frame) {
    // Deallocate frame buffer and point members to NULL
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

int transmit_file(dxwifi_transmitter* transmit, int fd) {
    assert_not_null(transmit);

    size_t nbytes   = 0;
    int status      = 0;
    dxwifi_frame data_frame;

    init_dxwifi_frame(&data_frame, transmit->block_size);

    while ((nbytes = read(fd, data_frame.payload, transmit->block_size)) > 0) {
        
        // Prepare data, FEC Encode etc. 
        status = pcap_inject(transmit->handle, data_frame.__frame, DXWIFI_HEADER_SIZE + nbytes);

        debug_assert_continue(status == 0, pcap_statustostr(status));

    }
    return 0;
}