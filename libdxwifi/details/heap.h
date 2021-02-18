/**
 * DxWiFi Packet Heap data structure
 */

#ifndef LIBDXWIFI_PACKET_HEAP_H
#define LIBDXWIFI_PACKET_HEAP_H

#include <stdbool.h>

#include <libdxwifi/receiver.h>

typedef struct {
    dxwifi_rx_packet*   packets;        /* Heap data                        */
    size_t              count;          /* Number of packets in the heap    */
    size_t              capacity;       /* Heap capacity                    */
    bool                is_max_heap;   
} dxwifi_packet_heap;

void init_packet_heap(dxwifi_packet_heap* heap, size_t capacity, bool is_max_heap);

void packet_heap_push(dxwifi_packet_heap* heap, dxwifi_rx_packet packet);

dxwifi_rx_packet packet_heap_pop(dxwifi_packet_heap* heap);

#endif // LIBDXWIFI_PACKET_HEAP_H
