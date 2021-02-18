/**
 * DxWifi Transmitter implementation
 */

#include <string.h>
#include <stdbool.h>

#include <poll.h>
#include <errno.h>
#include <unistd.h>
#include <endian.h>

#include <arpa/inet.h>

#include <libdxwifi/dxwifi.h>
#include <libdxwifi/transmitter.h>
#include <libdxwifi/details/utils.h>
#include <libdxwifi/details/logging.h>


typedef struct {
    uint32_t frame_count;
    uint32_t bytes_read;
    uint32_t bytes_sent;
} dxwifi_tx_stats;


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


static void construct_ieee80211_header( ieee80211_hdr* mac, ieee80211_frame_control fcntl, uint16_t duration_id, uint8_t* addr1, uint8_t* addr2, uint8_t* addr3) {
    debug_assert(mac && addr1 && addr2 && addr3);

    uint16_t frame_control = 0x00;

    set_bits16(&frame_control, IEEE80211_FCTL_VERS,     fcntl.protocol_version);
    set_bits16(&frame_control, IEEE80211_FCTL_FTYPE,    fcntl.type);
    set_bits16(&frame_control, IEEE80211_FCTL_STYPE,    fcntl.stype.data);
    set_bits16(&frame_control, IEEE80211_FCTL_TODS,     (fcntl.to_ds      ? IEEE80211_FCTL_TODS       : 0));
    set_bits16(&frame_control, IEEE80211_FCTL_FROMDS,   (fcntl.from_ds    ? IEEE80211_FCTL_FROMDS     : 0));
    set_bits16(&frame_control, IEEE80211_FCTL_RETRY,    (fcntl.retry      ? IEEE80211_FCTL_RETRY      : 0));
    set_bits16(&frame_control, IEEE80211_FCTL_PM,       (fcntl.power_mgmt ? IEEE80211_FCTL_PM         : 0));
    set_bits16(&frame_control, IEEE80211_FCTL_MOREDATA, (fcntl.more_data  ? IEEE80211_FCTL_MOREDATA   : 0));
    set_bits16(&frame_control, IEEE80211_FCTL_PROTECTED,(fcntl.wep        ? IEEE80211_FCTL_PROTECTED  : 0));
    set_bits16(&frame_control, IEEE80211_FCTL_ORDER,    (fcntl.order      ? IEEE80211_FCTL_ORDER      : 0));

    mac->frame_control = frame_control;

    mac->duration_id = htons(duration_id);

    memcpy(mac->addr1, addr1, IEEE80211_MAC_ADDR_LEN);
    memcpy(mac->addr2, addr2, IEEE80211_MAC_ADDR_LEN);
    memcpy(mac->addr3, addr3, IEEE80211_MAC_ADDR_LEN);

    mac->seq_ctrl = 0;
}


static const char* control_frame_type_to_str(dxwifi_control_frame_t type) {
    switch (type)
    {
    case CONTROL_FRAME_PREAMBLE:
        return "Preamble";
    
    case CONTROL_FRAME_END_OF_TRANSMISSION:
        return "EOT";

    default:
        return "Unknown control type";
    }
}


static void send_control_frame(dxwifi_transmitter* tx, dxwifi_tx_frame* data_frame, dxwifi_control_frame_t type) {
    debug_assert(tx && tx->__handle && data_frame && data_frame->__frame);

    uint8_t control_data[DXWIFI_FRAME_CONTROL_DATA_SIZE];

    memset(control_data, type, DXWIFI_FRAME_CONTROL_DATA_SIZE);

    memcpy(data_frame->payload, control_data, DXWIFI_FRAME_CONTROL_DATA_SIZE);

    // TODO add redundancy and send multiple times?
    int status = pcap_inject(tx->__handle, data_frame->__frame, DXWIFI_TX_HEADER_SIZE + sizeof(control_data) + IEEE80211_FCS_SIZE);

    log_info("%s Frame Sent: %d", control_frame_type_to_str(type), status);
    log_hexdump(data_frame->__frame, DXWIFI_TX_HEADER_SIZE + sizeof(control_data) + IEEE80211_FCS_SIZE);
}


static void log_tx_configuration(const dxwifi_transmitter* tx) {
    log_info(
            "DxWifi Transmitter Settings\n"
            "\tDevice:        %s\n"
            "\tBlock Size:    %ld\n"
            "\tData Rate:     %dMbps\n"
            "\tRTAP flags:    0x%x\n"
            "\tRTAP Tx flags: 0x%x\n",
            tx->device,
            tx->block_size,
            tx->rtap_rate,
            tx->rtap_flags,
            tx->rtap_tx_flags
    );
}


static void log_frame_stats(const dxwifi_tx_frame* frame, size_t bytes_read, size_t bytes_sent, int frame_count) {
    log_debug("Frame: %d - (Read: %ld, Sent: %ld)", frame_count, bytes_read, bytes_sent);
    log_hexdump(frame->__frame, DXWIFI_TX_HEADER_SIZE + bytes_read + IEEE80211_FCS_SIZE);
}

static void log_tx_stats(dxwifi_tx_stats stats) {
    log_info(
        "Transmission Stats\n"
        "\tTotal Bytes Read:    %d\n"
        "\tTotal Bytes Sent:    %d\n"
        "\tTotal Frames Sent:   %d\n",
        stats.bytes_read,
        stats.bytes_sent,
        stats.frame_count
        );
}


void init_transmitter(dxwifi_transmitter* tx) {
    debug_assert(tx);

    char err_buff[PCAP_ERRBUF_SIZE];

    tx->__handle = pcap_open_live(
                        tx->device, 
                        SNAPLEN_MAX, 
                        true, 
                        DXWIFI_DFLT_PACKET_BUFFER_TIMEOUT, 
                        err_buff
                    );

    // Hard assert here because if pcap fails it's all FUBAR anyways
    assert_M(tx->__handle != NULL, err_buff);

    log_tx_configuration(tx);
}


void close_transmitter(dxwifi_transmitter* transmitter) {
    debug_assert(transmitter && transmitter->__handle);

    pcap_close(transmitter->__handle);

    log_info("DxWifi Transmitter closed");
}


int start_transmission(dxwifi_transmitter* tx, int fd) {
    debug_assert(tx && tx->__handle);

    int status          = 0;
    int nbytes          = 0;
    size_t blocksize    = tx->block_size;

    struct pollfd   request     = { 0 };
    dxwifi_tx_stats tx_stats    = { 0 };
    dxwifi_tx_frame data_frame  = { 0 };

    request.fd = fd;

    request.events = POLLIN; // Listen for read events only

    init_dxwifi_tx_frame(&data_frame, blocksize);

    construct_radiotap_header(data_frame.radiotap_hdr, tx->rtap_flags, tx->rtap_rate, tx->rtap_tx_flags);

    construct_ieee80211_header(data_frame.mac_hdr, tx->fctl, DXWIFI_TX_DURATION_ID, tx->addr1, tx->addr2, tx->addr3);

    log_info("Starting transmission...");

    tx->__activated = true;

    send_control_frame(tx, &data_frame, CONTROL_FRAME_PREAMBLE);

    do
    {
        status = poll(&request, 1, tx->transmit_timeout * 1000);

        if(status == 0) {
            log_info("Transmitter timeout occured");
            tx->__activated = false;
        }
        else if(status < 0) {
            if( tx->__activated) {
                log_error("Error occured: %s", strerror(errno));
            }
            tx->__activated = false;
        }
        else {
            nbytes = read(fd, data_frame.payload, tx->block_size);
            if(nbytes > 0) {
                status = pcap_inject(tx->__handle, data_frame.__frame, DXWIFI_TX_HEADER_SIZE + nbytes + IEEE80211_FCS_SIZE);

                debug_assert_continue(status > 0, "Injection failure: %s", pcap_statustostr(status));

                tx_stats.bytes_read  += nbytes;
                tx_stats.bytes_sent  += status;
                tx_stats.frame_count += 1;

                log_frame_stats(&data_frame, nbytes, status, tx_stats.frame_count);
            }
        }
    } while (tx->__activated && nbytes > 0);

    send_control_frame(tx, &data_frame, CONTROL_FRAME_END_OF_TRANSMISSION);

    log_tx_stats(tx_stats);
    
    teardown_dxwifi_frame(&data_frame);

    return tx_stats.frame_count;
}

void stop_transmission(dxwifi_transmitter* tx) {
    if(tx) {
        log_info("DxWifi Transmission stopped");
        tx->__activated = false;
    }
}
