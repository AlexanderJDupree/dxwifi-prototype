/**
 * DxWiFi Packet Heap implementation
 */

#include <libdxwifi/details/heap.h>
#include <libdxwifi/details/utils.h>
#include <libdxwifi/details/logging.h>


typedef bool (*comparator)(const dxwifi_rx_packet* lhs, const dxwifi_rx_packet* rhs);


static inline bool less_than(const dxwifi_rx_packet* lhs, const dxwifi_rx_packet* rhs) {
    return lhs->frame_number < rhs->frame_number;
}


static inline bool greater_than(const dxwifi_rx_packet* lhs, const dxwifi_rx_packet* rhs) {
    return lhs->frame_number > rhs->frame_number;
}


static inline size_t parent(size_t i) { return (i-1)/2;   }
static inline size_t left(size_t i)   { return (2*i + 1); }
static inline size_t right(size_t i)  { return (2*i + 2); }


static void swap(dxwifi_rx_packet* a, dxwifi_rx_packet* b) {
    dxwifi_rx_packet temp = *a;
    *a = *b;
    *b = temp;
}


static void heapify(dxwifi_packet_heap* heap, comparator compare, size_t i) {
    size_t l = left(i);
    size_t r = right(i);
    size_t new_index = i;

    if(l < heap->heap_size && compare(&heap->packets[l], &heap->packets[i])) {
        new_index = l;
    }
    if(r < heap->heap_size && compare(&heap->packets[r], &heap->packets[new_index])) {
        new_index = r;
    }
    if(new_index != i) {
        swap(&heap->packets[i], &heap->packets[new_index]);
        heapify(heap, compare, new_index);
    }
}


void init_packet_heap(dxwifi_packet_heap* heap, size_t capacity, bool is_max_heap) {
    debug_assert(heap);

    heap->count         = 0; 
    heap->heap_size     = capacity;
    heap->is_max_heap   = is_max_heap;

    heap->packets = calloc(capacity, sizeof(dxwifi_rx_packet));
    assert_M(heap->packets, "Failed to allocated heap");
}


void packet_heap_push(dxwifi_packet_heap* heap, dxwifi_rx_packet packet) {

    comparator compare = (heap->is_max_heap) ? greater_than : less_than;

    if(heap->count < heap->heap_size) {
        size_t i = heap->count;
        heap->packets[heap->count++] = packet;

        // Sift from the bottom up
        while(i != 0 && compare(&heap->packets[i], &heap->packets[parent(i)])) {
            swap(&heap->packets[i], &heap->packets[parent(i)]);
            i = parent(i);
        }
    }
    else {
        log_error("Packet Heap is full");
    }
}


dxwifi_rx_packet packet_heap_pop(dxwifi_packet_heap* heap) {

    dxwifi_rx_packet packet = { 0 };

    comparator compare = (heap->is_max_heap) ? greater_than : less_than;

    if (heap->count > 0) {

        packet = heap->packets[0];

        swap(&heap->packets[0], &heap->packets[--heap->count]);

        heapify(heap, compare, 0);
    }

    return packet;
}