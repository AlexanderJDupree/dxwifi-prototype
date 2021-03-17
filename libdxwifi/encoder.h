/**
 *  encoder.h - API to FEC encode data 
 * 
 */

#ifndef LIBDXWIFI_ENCODER_H
#define LIBDXWIFI_ENCODER_H

#include <stdlib.h>
#include <stdint.h>

/************************
 *  Includes
 ***********************/


/************************
 *  Constants
 ***********************/

/************************
 *  Data structures
 ***********************/

typedef struct __attribute__((packed)) {
    uint32_t esi;
    uint32_t n;
    uint32_t k;
    uint32_t crc;
    uint32_t symbol_size;
} dxwifi_oti; 

typedef struct __dxwifi_encoder dxwifi_encoder;

/************************
 *  Functions
 ***********************/

dxwifi_encoder* init_encoder(unsigned k, unsigned n, unsigned symbol_size);

void close_encoder(dxwifi_encoder* encoder);

size_t dxwifi_encode(dxwifi_encoder* encoder, void* message, size_t msglen, void** out);

#endif // LIBDXWIFI_ENCODER_H