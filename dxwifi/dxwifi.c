/**
 * DxWifi project library implementations
 */

#include <stdbool.h>

#include <dxwifi/utils.h>
#include <dxwifi/dxwifi.h>

void init_dxwifi_frame(dxwifi_frame* frame) {
    // 1. 
    // Allocate the frame data to the block size + size of all the headers
    // Do we want to over allocated the frame data? have some room to play with?
    // Probably not. 

    // 2. Point the frame fields to the correct spots in the frame data buffer

    // 3. Add the correct data to the headers
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
    dxwifi_frame data_frame;
    return 0;
}