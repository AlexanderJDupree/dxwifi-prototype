/**
 * DxWifi project library implementations
 */

#include <time.h>
#include <string.h>
#include <stdbool.h>

#include <poll.h>
#include <errno.h>
#include <unistd.h>
#include <endian.h>

#include <arpa/inet.h>

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


static void log_tx_configuration(const dxwifi_transmitter* tx) {
    log_info(
            "DxWifi Transmitter Settings\n"
            "\tDevice:        %s\n"
            "\tData Rate:     %dMbps\n"
            "\tRTAP flags:    0x%x\n"
            "\tRTAP Tx flags: 0x%x\n",
            tx->device,
            tx->rtap_rate,
            tx->rtap_flags,
            tx->rtap_tx_flags
    );
}


static void log_rx_configuration(const dxwifi_receiver* rx) {
    int datalink = pcap_datalink(rx->__handle);
    log_info(
            "DxWifi Receiver Settings\n"
            "\tDevice:                   %s\n"
            "\tFilter:                   %s\n"
            "\tOptimize:                 %d\n"
            "\tSnapshot Length:          %d\n"
            "\tPacket Buffer Timeout:    %dms\n"
            "\tDatalink Type:            %s\n",
            rx->device,
            rx->filter,
            rx->optimize,
            rx->snaplen,
            rx->packet_buffer_timeout,
            pcap_datalink_val_to_description(datalink)
    );
}


static void log_tx_stats(const dxwifi_tx_frame* frame, size_t bytes_read, size_t bytes_sent, int frame_count) {
    log_debug("Frame: %d - (Bytes Read, Bytes Sent) = (%ld, %ld)", frame_count, bytes_read, bytes_sent);
    log_hexdump(frame->__frame, DXWIFI_TX_HEADER_SIZE + bytes_read + IEEE80211_FCS_SIZE);
}

static void log_rx_stats(const dxwifi_receiver* rx, const struct pcap_pkthdr* pkt_stats, const uint8_t* data) {

    char timestamp[64];
    struct tm *time;
    struct pcap_stat capture_stats;

    time = gmtime(&pkt_stats->ts.tv_sec);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", time);

    if(pcap_stats(rx->__handle, &capture_stats) == PCAP_ERROR) {
        log_debug("Failed to gather capture statistics");
    }
    else {
        // WARNING! capture stats are inconsistent from platform to platform
        log_debug(
            "Packets Received: %d        "
            "Packets Dropped (Kernel): %d        "
            "Packets Dropped (NIC): %d",
            capture_stats.ps_recv,
            capture_stats.ps_drop,
            capture_stats.ps_ifdrop
            );
    }
    log_debug("(%s) - (Capture Length, Packet Length) = (%d, %d)", timestamp, pkt_stats->caplen, pkt_stats->len);
    log_hexdump(data, pkt_stats->caplen);
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

    log_tx_configuration(tx);
}


void init_receiver(dxwifi_receiver* rx) {
    debug_assert(rx && rx->filter);

    int status = 0;
    struct bpf_program filter;
    char err_buff[PCAP_ERRBUF_SIZE];

    rx->__handle = pcap_open_live(
                        rx->device,
                        rx->snaplen,
                        true, 
                        rx->packet_buffer_timeout,
                        err_buff
                    );
    assert_M(rx->__handle != NULL, err_buff);

    status = pcap_set_datalink(rx->__handle, DLT_IEEE802_11_RADIO);
    assert_M(status != PCAP_ERROR, "Failed to set datalink: %s", pcap_statustostr(status));

    status = pcap_setnonblock(rx->__handle, true, err_buff);
    assert_M(status != PCAP_ERROR, "Failed to set nonblocking mode: %s", err_buff);

    status = pcap_compile(rx->__handle, &filter, rx->filter, rx->optimize, PCAP_NETMASK_UNKNOWN);
    assert_M(status != PCAP_ERROR, "Failed to compile filter %s: %s", rx->filter, pcap_statustostr(status));

    status = pcap_setfilter(rx->__handle, &filter);
    assert_M(status != PCAP_ERROR, "Failed to set filter: %s", pcap_statustostr(status));

    log_rx_configuration(rx);
}


void close_transmitter(dxwifi_transmitter* transmitter) {
    debug_assert(transmitter && transmitter->__handle);

    pcap_close(transmitter->__handle);

    log_info("DxWifi Transmitter closed");
}


void close_receiver(dxwifi_receiver* receiver) {
    debug_assert(receiver && receiver->__handle);

    pcap_close(receiver->__handle);

    log_info("DxWifi Receiver closed");
}


int transmit_file(dxwifi_transmitter* tx, int fd) {
    debug_assert(tx && tx->__handle);

    size_t blocksize    = tx->block_size;
    size_t nbytes       = 0;
    int status          = 0;
    int frame_count     = 0;

    dxwifi_tx_frame data_frame;

    init_dxwifi_tx_frame(&data_frame, blocksize);

    construct_radiotap_header(data_frame.radiotap_hdr, tx->rtap_flags, tx->rtap_rate, tx->rtap_tx_flags);

    construct_ieee80211_header(data_frame.mac_hdr, tx->fctl, DXWIFI_TX_DURATION_ID, tx->addr1, tx->addr2, tx->addr3);

    // TODO: poll fd to see if there's any data to even read, no need to block waiting on a read
    while((nbytes = read(fd, data_frame.payload, tx->block_size)) > 0) {

        status = pcap_inject(tx->__handle, data_frame.__frame, DXWIFI_TX_HEADER_SIZE + nbytes + IEEE80211_FCS_SIZE);

        log_tx_stats(&data_frame, nbytes, status, ++frame_count);

        debug_assert_continue(status > 0, "Injection failure: %s", pcap_statustostr(status));
    }

    teardown_dxwifi_frame(&data_frame);

    return 0; // TODO accumulate stats into some sort of struct and return that
}

static void process_frame(uint8_t* args, const struct pcap_pkthdr* pkt_stats, const uint8_t* frame) {
    int fd = *args;

    const struct ieee80211_radiotap_header* radiotap_hdr = (struct ieee80211_radiotap_header*) frame;

    //const ieee80211_hdr* mac_hdr = (ieee80211_hdr*) (frame + radiotap_hdr->it_len);

    size_t payload_size = pkt_stats->caplen - radiotap_hdr->it_len - sizeof(ieee80211_hdr) - IEEE80211_FCS_SIZE;
    const uint8_t* payload = frame + radiotap_hdr->it_len + sizeof(ieee80211_hdr);


    write(fd, payload, payload_size);
};

int receiver_activate_capture(dxwifi_receiver* rx, int fd) {
    debug_assert(rx && rx->__handle);

    int status = 0;
    struct pollfd request;

    request.fd = pcap_get_selectable_fd(rx->__handle);

    request.events = POLLIN;

    rx->__activated = true;
    // TODO loop control
    while(rx->__activated) {

        status = poll(&request, 1, 10000);
        if (status == 0) {
            log_warning("Receiver timeout occured");
            rx->__activated = false;
        }
        else if (status < 0) {
            log_error("Error occured: %s", strerror(errno));
            rx->__activated = false;
        }
        else {
            pcap_dispatch(rx->__handle, 1, process_frame, (uint8_t*)&fd);
        }
    }

    return status;
}


void receiver_stop_capture(dxwifi_receiver* receiver) {
    receiver->__activated = false;
}
