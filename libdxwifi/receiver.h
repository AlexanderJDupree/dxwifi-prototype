/**
 * DxWifi Receiver handles capturing data frames and unpacking the payload data
 */

#ifndef LIBDXWIFI_RECEIVER_H
#define LIBDXWIFI_RECEIVER_H

#include <pcap.h>

#include <libdxwifi/dxwifi.h>


/************************
 *  Constants
 ***********************/

#define DXWIFI_RX_PACKET_BUFFER_SIZE_MIN 1024
#define DXWIFI_RX_PACKET_BUFFER_SIZE_MAX (1024 * 1024)
#define DXWIFI_RX_PACKET_HEAP_SIZE ((DXWIFI_RX_PACKET_BUFFER_SIZE_MAX / DXWIFI_BLOCK_SIZE_MIN) + 1)


/************************
 *  Data structures
 ***********************/

typedef struct {
    const char*     device;                 /* 802.11 interface name                        */
    unsigned        dispatch_count;         /* Number of packets to process at a time       */
    unsigned        capture_timeout;        /* Number of seconds to wait for a packet       */
    size_t          packet_buffer_size;     /* Size of the packet buffer                    */

    // https://www.tcpdump.org/manpages/pcap.3pcap.html
    const char*     filter;                 /* BPF Program string                           */
    bool            optimize;               /* Optimize compiled Filter?                    */
    int             snaplen;                /* Snapshot length in bytes                     */
    int             packet_buffer_timeout;

    volatile bool   __activated;            /* Currently capturing packets?                 */
    pcap_t*         __handle;               /* Session handle for PCAP                      */
} dxwifi_receiver;


typedef struct {
    uint32_t    frame_number;               /* The number of the frame data was sent with   */
    uint8_t*    data;                       /* Frame payload data                           */
    size_t      size;                       /* Size of the payload data                     */
    bool        crc_valid;                  /* Was the attached crc correct?                */
} dxwifi_rx_packet;


/************************
 *  Functions
 ***********************/

void init_receiver(dxwifi_receiver* receiver);

int receiver_activate_capture(dxwifi_receiver* receiver, int fd);

void receiver_stop_capture(dxwifi_receiver* receiver);

void close_receiver(dxwifi_receiver* receiver);


#endif // LIBDXWIFI_RECEIVER_H
