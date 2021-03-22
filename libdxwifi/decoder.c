/**
 *  decoder.c - See decoder.h
 * 
 */

#include <arpa/inet.h>

#include <of_openfec_api.h>

#include <libdxwifi/decoder.h>
#include <libdxwifi/encoder.h>
#include <libdxwifi/details/crc32.h>
#include <libdxwifi/details/utils.h>
#include <libdxwifi/details/assert.h>
#include <libdxwifi/details/logging.h>

#define member_size(type, member) sizeof(((type *)0)->member)

struct __dxwifi_decoder { // TODO, this is literally the same definition of the dxwifi_encoder
    uint32_t k;
    uint32_t n;
    of_session_t* openfec_session;
};


static void log_decoder_config(dxwifi_decoder* decoder) {
    log_info(
        "DxWiFi Decoder\n"
        "\tK:   %d\n"
        "\tN:   %d\n",
        decoder->k,
        decoder->n
    );
}


static const dxwifi_oti* find_valid_oti(void* encoded, size_t msglen) {
    for(size_t i = 0; i < msglen; i += DXWIFI_LDPC_FRAME_SIZE) {
        dxwifi_oti* oti = encoded + i;
        void* message   = encoded + i + sizeof(dxwifi_oti);
        uint32_t crc    = crc32(message, DXWIFI_FEC_SYMBOL_SIZE);

        if(crc == ntohl(oti->crc)) { // Found valid OTI
            log_debug("Valid OTI found at Byte: %u", i);
            return oti;
        }

    }
    return NULL;
}


dxwifi_decoder* init_decoder(void* encoded, size_t msglen) {

    of_status_t status = OF_STATUS_OK;

    const dxwifi_oti* oti = find_valid_oti(encoded, msglen);
    assert_M(oti, "Failed to find a valid Object Tranmission Information (OTI) header");

    dxwifi_decoder* decoder = calloc(1, sizeof(dxwifi_decoder));
    assert_M(decoder, "Failed to allocate space for decoder");

    uint32_t n = ntohl(oti->n);
    uint32_t k = ntohl(oti->k);

    of_ldpc_parameters_t codec_params = {
        .nb_source_symbols      = k,
        .nb_repair_symbols      = n - k,
        .encoding_symbol_length = DXWIFI_FEC_SYMBOL_SIZE,
        .prng_seed              = rand(), // TODO, does this need to match the encoder prng_seed?
        .N1                     = (n-k) > DXWIFI_LDPC_N1_MAX ? DXWIFI_LDPC_N1_MAX : (n-k) 
    };
    assert_M(codec_params.N1 >= DXWIFI_LDPC_N1_MIN, "N - K must be >= %d", DXWIFI_LDPC_N1_MIN);

    decoder->n = n;
    decoder->k = k;

    status = of_create_codec_instance(&decoder->openfec_session, OF_CODEC_LDPC_STAIRCASE_STABLE, OF_DECODER, 2); // TODO magic number
    assert_M(status == OF_STATUS_OK, "Failed to initialize OpenFEC session");

    status = of_set_fec_parameters(decoder->openfec_session, (of_parameters_t*) &codec_params);
    assert_M(status == OF_STATUS_OK, "Failed to set codec parameters");

    log_decoder_config(decoder);

    return decoder;
}


void close_decoder(dxwifi_decoder* decoder) {
    if(decoder) {
        of_release_codec_instance(decoder->openfec_session);

        free(decoder);

        decoder = NULL;
    }
}


size_t dxwifi_decode(dxwifi_decoder* decoder, void* encoded_message, size_t msglen, void** out) {
    debug_assert(decoder && decoder->openfec_session && out);

    void* symbol_table[decoder->n];
    of_status_t status = OF_STATUS_OK;

    for(size_t i = 0; i < msglen; i += DXWIFI_LDPC_FRAME_SIZE) {
        dxwifi_oti* oti = encoded_message + i;

        log_trace("esi=%d, i=%d", ntohl(oti->esi), i);

        of_decode_with_new_symbol(
            decoder->openfec_session, 
            encoded_message + i + sizeof(dxwifi_oti), 
            ntohl(oti->esi)
        );
        // TODO we can shorten the loop by checking if the decoding is finished after processing
        // K symbols
    }
    if(!of_is_decoding_complete(decoder->openfec_session)) {
        status = of_finish_decoding(decoder->openfec_session);
        if(status != OF_STATUS_OK) {
            log_error("Failed to decode message"); // TODO, how did we fail to decode msg?
            return 0;
        }
    }

    status = of_get_source_symbols_tab(decoder->openfec_session, symbol_table);
    if(status != OF_STATUS_OK) {
        log_error("Failed to retrieve source symbols");
        return 0;
    }

    void* decoded_msg = calloc(decoder->k, DXWIFI_FEC_SYMBOL_SIZE);
    assert_M(decoded_msg, "Failed to allocated memory for decoded message");

    for(size_t esi = 0; esi < decoder->k; ++esi) {
        void* symbol = offset(decoded_msg, esi, DXWIFI_FEC_SYMBOL_SIZE);
        memcpy(symbol, symbol_table[esi], DXWIFI_FEC_SYMBOL_SIZE);
    }
    *out = decoded_msg;
    return decoder->k * DXWIFI_FEC_SYMBOL_SIZE;
}
