/**
 * DxWiFi Receiver implementation
 */

#include <time.h>
#include <string.h>
#include <stdbool.h>

#include <poll.h>
#include <errno.h>
#include <unistd.h>

#include <libdxwifi/receiver.h>
#include <libdxwifi/details/utils.h>
#include <libdxwifi/details/logging.h>
#include <libdxwifi/details/ieee80211.h>


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


void close_receiver(dxwifi_receiver* receiver) {
    debug_assert(receiver && receiver->__handle);

    pcap_close(receiver->__handle);

    log_info("DxWifi Receiver closed");
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
    assert_M(request.fd > 0, "Receiver handle cannot be polled");

    request.events = POLLIN; // Listen for read events only

    rx->__activated = true;
    while(rx->__activated) {

        status = poll(&request, 1, rx->capture_timeout * 1000);
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
    if(receiver) {
        log_info("DxWiFi Receiver closed");
        receiver->__activated = false;
    }
}
