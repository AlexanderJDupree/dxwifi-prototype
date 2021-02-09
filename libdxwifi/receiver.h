/**
 * DxWifi Receiver handles capturing data frames and unpacking the payload data
 */

#ifndef LIBDXWIFI_RECEIVER_H
#define LIBDXWIFI_RECEIVER_H

#include <pcap.h>

/************************
 *  Constants
 ***********************/


/************************
 *  Data structures
 ***********************/

typedef struct {
    const char*     device;                 /* 802.11 interface name                        */
    int             capture_timeout;        /* Number of seconds to wait for a packet       */

    // https://www.tcpdump.org/manpages/pcap.3pcap.html
    const char*     filter;                 /* BPF Program string                           */
    bool            optimize;               /* Optimize compiled Filter?                    */
    int             snaplen;                /* Snapshot length in bytes                     */
    int             packet_buffer_timeout;

    volatile bool   __activated;            /* Currently capturing packets?                 */
    pcap_t*         __handle;               /* Session handle for PCAP                      */
} dxwifi_receiver;


/************************
 *  Functions
 ***********************/

void init_receiver(dxwifi_receiver* receiver);

int receiver_activate_capture(dxwifi_receiver* receiver, int fd);

void receiver_stop_capture(dxwifi_receiver* receiver);

void close_receiver(dxwifi_receiver* receiver);


#endif // LIBDXWIFI_RECEIVER_H
