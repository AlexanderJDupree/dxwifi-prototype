/**
 * DxWiFi Receiver implementation
 */

#include <time.h>
#include <string.h>
#include <stdbool.h>

#include <poll.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <libdxwifi/dxwifi.h>
#include <libdxwifi/receiver.h>
#include <libdxwifi/details/heap.h>
#include <libdxwifi/details/utils.h>
#include <libdxwifi/details/logging.h>
#include <libdxwifi/details/ieee80211.h>


typedef struct {
    dxwifi_packet_heap  heap;
    uint8_t*            packet_buffer;
    size_t              index;
    size_t              count;
    size_t              packet_buffer_size;
    bool                eot_reached;
    int                 fd;
} frame_controller;


static void log_rx_configuration(const dxwifi_receiver* rx) {
    int datalink = pcap_datalink(rx->__handle);
    log_info(
            "DxWifi Receiver Settings\n"
            "\tDevice:                   %s\n"
            "\tPacket Buffer Size:       %ld\n"
            "\tFilter:                   %s\n"
            "\tOptimize:                 %d\n"
            "\tSnapshot Length:          %d\n"
            "\tPacket Buffer Timeout:    %dms\n"
            "\tDatalink Type:            %s\n",
            rx->device,
            rx->packet_buffer_size,
            rx->filter,
            rx->optimize,
            rx->snaplen,
            rx->packet_buffer_timeout,
            pcap_datalink_val_to_description(datalink)
    );
}


static void log_frame_stats(const struct pcap_pkthdr* frame_stats, const uint8_t* data) {

    char timestamp[64];
    struct tm *time;

    time = gmtime(&frame_stats->ts.tv_sec);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", time);

    log_debug(
        "(%s:%d) - (Capture Length, Packet Length) = (%d, %d)", 
        timestamp, 
        frame_stats->caplen, 
        frame_stats->len
        );
    log_hexdump(data, frame_stats->caplen);
}


static void log_capture_stats(dxwifi_receiver* rx) {

    struct pcap_stat capture_stats;

    if(pcap_stats(rx->__handle, &capture_stats) == PCAP_ERROR) {
        log_info("Failed to gather capture statistics");
    }
    else {
        // WARNING! capture stats are inconsistent from platform to platform
        log_info(
            "Receiver Capture Stats\n"
            "\tPackets Received: %d\n"
            "\tPackets Dropped (Kernel): %d\n"
            "\tPackets Dropped (NIC): %d",
            capture_stats.ps_recv,
            capture_stats.ps_drop,
            capture_stats.ps_ifdrop
            );
    }
}


static void init_frame_controller(frame_controller* fc, size_t buffsize) {
    debug_assert(fc && buffsize > 0);

    fc->index = 0;
    fc->count = 0;
    fc->eot_reached = false;
    fc->packet_buffer_size = buffsize;
    fc->packet_buffer = calloc(fc->packet_buffer_size, sizeof(uint8_t));
    assert_M(fc->packet_buffer, "Failed to allocated Packet Buffer of size: %ld", fc->packet_buffer_size);

    init_packet_heap(&fc->heap, DXWIFI_RX_PACKET_HEAP_SIZE, false);
}


static void teardown_frame_controller(frame_controller* fc) {
    debug_assert(fc);

    free(fc->packet_buffer);
    fc->packet_buffer = NULL;
    fc->packet_buffer_size = 0;
    fc->index = 0;
}


static void dump_packet(frame_controller* fc, const uint8_t* payload, size_t payload_size, const ieee80211_hdr* mac_hdr) {

    dxwifi_rx_packet packet;

    memcpy(fc->packet_buffer + fc->index, payload, payload_size);

    packet.frame_number = ntohl(*(uint32_t*)(mac_hdr->addr1 + 2));
    packet.data         = fc->packet_buffer + fc->index;
    packet.size         = payload_size;
    packet.crc_valid    = false; // TODO verify CRC;

    fc->index += payload_size;
    fc->count += 1;

    log_info("Frame number: %d", packet.frame_number);

    packet_heap_push(&fc->heap, packet);
}


static void dump_packet_buffer(frame_controller* fc) {
    dxwifi_rx_packet packet;
    while(fc->heap.count > 0) {
        packet = packet_heap_pop(&fc->heap);
        write(fc->fd, packet.data, packet.size);
    }
    fc->index = 0;
}


static dxwifi_control_frame_t check_frame_control(const uint8_t* payload, size_t payload_size, float check_threshold) {
    debug_assert(payload && payload_size > 0);

    unsigned eot = 0;
    unsigned preamble = 0;
    dxwifi_control_frame_t type = CONTROL_FRAME_NONE;

    if( payload_size <= DXWIFI_FRAME_CONTROL_DATA_SIZE ) {
        for (size_t i = 0; i < payload_size; i++)
        {
            switch (payload[i])
            {
            case CONTROL_FRAME_PREAMBLE:
                ++preamble;
                break;

            case CONTROL_FRAME_END_OF_TRANSMISSION:
                ++eot;
                break;
            
            default:
                break;
            }
        }
        if ( (eot / payload_size) > check_threshold) {
            type = CONTROL_FRAME_END_OF_TRANSMISSION;
        }
        else if ( (preamble / payload_size ) > check_threshold) {
            type = CONTROL_FRAME_PREAMBLE;
        }
    }
    return type;
}

static void handle_frame_control(frame_controller* fc, dxwifi_control_frame_t type) {
    switch (type)
    {
    case CONTROL_FRAME_PREAMBLE:
        // TODO: handle cases where we receive a preamble when we weren't expecting it
        log_info("Uplink established!");
        break;

    case CONTROL_FRAME_END_OF_TRANSMISSION:
        log_info("End-Of-Transmission signalled");
        fc->eot_reached = true;
        break;
    
    default:
        debug_assert_always("Unknown control type");
        break;
    }
}


static void process_frame(uint8_t* args, const struct pcap_pkthdr* pkt_stats, const uint8_t* frame) {

    frame_controller* fc = (frame_controller*)args;

    const struct ieee80211_radiotap_header* radiotap_hdr = (struct ieee80211_radiotap_header*) frame;

    const ieee80211_hdr* mac_hdr = (ieee80211_hdr*) (frame + radiotap_hdr->it_len);

    size_t payload_size = pkt_stats->caplen - radiotap_hdr->it_len - sizeof(ieee80211_hdr) - IEEE80211_FCS_SIZE;

    const uint8_t* payload = frame + radiotap_hdr->it_len + sizeof(ieee80211_hdr);

    dxwifi_control_frame_t control_frame = check_frame_control(payload, payload_size, DXWIFI_FRAME_CONTROL_CHECK_THRESHOLD);

    if ( control_frame != CONTROL_FRAME_NONE ) {
        handle_frame_control(fc, control_frame);
    }
    else {
        // Buffer is full, write it out and reset position
        if( fc->index + payload_size >= fc->packet_buffer_size ) {
            dump_packet_buffer(fc);
        }

        dump_packet(fc, payload, payload_size, mac_hdr);
    }

    // TODO add radiotap and mac header to stats
    log_frame_stats(pkt_stats, frame);
};


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


int receiver_activate_capture(dxwifi_receiver* rx, int fd) {
    debug_assert(rx && rx->__handle);

    int status = 0;
    int num_packets = 0;

    frame_controller fc;
    struct pollfd request;

    fc.fd = fd;
    init_frame_controller(&fc, rx->packet_buffer_size);

    request.fd = pcap_get_selectable_fd(rx->__handle);
    assert_M(request.fd > 0, "Receiver handle cannot be polled");

    request.events = POLLIN; // Listen for read events only

    log_info("Starting packet capture...");
    rx->__activated = true;
    while(rx->__activated) {

        status = poll(&request, 1, rx->capture_timeout * 1000);
        if (status == 0) {
            log_info("Receiver timeout occured");
            rx->__activated = false;
        }
        else if (status < 0) {
            if(rx->__activated) {
                log_error("Error occured: %s", strerror(errno));
            }
            rx->__activated = false;
        }
        else {
            num_packets += pcap_dispatch(rx->__handle, rx->dispatch_count, process_frame, (uint8_t*)&fc);
        }

    }

    dump_packet_buffer(&fc);

    teardown_frame_controller(&fc);

    log_capture_stats(rx);

    return num_packets;
}


void receiver_stop_capture(dxwifi_receiver* receiver) {
    if(receiver) {
        log_info("DxWiFi Receiver capture ended");
        receiver->__activated = false;
    }
}
