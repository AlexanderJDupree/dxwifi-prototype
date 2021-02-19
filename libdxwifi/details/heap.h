/**
 * DxWiFi Binary Heap data structure
 */

#ifndef LIBDXWIFI_BINARY_HEAP_H
#define LIBDXWIFI_BINARY_HEAP_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


typedef bool (*comparator)(const uint8_t* lhs, const uint8_t* rhs);


typedef struct {
    uint8_t*    tree;       /* Heap data                                    */
    size_t      count;      /* Number of elements currently in the heap     */
    size_t      capacity;   /* Number of elements that can fit into tree    */
    size_t      step_size;  /* Size of each element in the heap             */
    comparator  compare;    /* Ordering function                            */
} binary_heap;


void init_heap(binary_heap* heap, size_t capacity, size_t step_size, comparator compare);

void teardown_heap(binary_heap* heap);

void heap_push(binary_heap* heap, const void* packet);

bool heap_pop(binary_heap* heap, void* out);

#endif // LIBDXWIFI_BINARY_HEAP_H
