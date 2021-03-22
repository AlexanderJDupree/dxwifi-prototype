/**
 *  decoder.c - See decoder.h
 * 
 */

#include <arpa/inet.h>

#include <libdxwifi/decoder.h>
#include <libdxwifi/encoder.h>
#include <libdxwifi/details/crc32.h>
#include <libdxwifi/details/utils.h>

#define member_size(type, member) sizeof(((type *)0)->member)

struct __dxwifi_decoder {
    uint32_t n;
    uint32_t k;
};


dxwifi_decoder* init_decoder(void* encoded, size_t msglen) {

    // Find valid OTI

        // Encoder should enforce the symbol size to be OTI aligned

    // Initialize OpenFEC
    return NULL;
}
