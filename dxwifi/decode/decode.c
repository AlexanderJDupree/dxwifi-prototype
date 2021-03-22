/**
 *  encode.c - FEC Encoding program
 * 
 */


#include <math.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <arpa/inet.h>

#include <of_openfec_api.h>

#include <dxwifi/decode/cli.h>

#include <libdxwifi/encoder.h>
#include <libdxwifi/details/utils.h>
#include <libdxwifi/details/assert.h>
#include <libdxwifi/details/logging.h>

void decode_file(cli_args* args);
void decode_stream(cli_args* args);

int main(int argc, char** argv) {
    cli_args args = {
        .file_in    = NULL,
        .file_out   = NULL,
        .verbosity  = DXWIFI_LOG_INFO,
        .quiet      = false
    };

    parse_args(argc, argv, &args);

    set_log_level(DXWIFI_LOG_ALL_MODULES, args.verbosity);

    if(args.file_in) {
        decode_file(&args);
    }
    else {
        decode_stream(&args);
    }

    exit(0);
}


void decode_file(cli_args* args) {

    // Setup File In / File Out
    int fd_in = open(args->file_in, O_RDWR);
    assert_M(fd_in > 0, "Failed to open file: %s - %s", args->file_in, strerror(errno));

    int open_flags  = O_WRONLY | O_CREAT | O_TRUNC;
    mode_t mode     = S_IRUSR  | S_IWUSR | S_IROTH | S_IWOTH; 

    int fd_out      = args->file_out ? open(args->file_out, open_flags, mode) : STDOUT_FILENO;
    assert_M(fd_out > 0, "Failed to open file: %s - %s", args->file_out, strerror(errno));

    off_t file_size = get_file_size(args->file_in);

    void* file_data = mmap(NULL, file_size, PROT_READ, MAP_SHARED, fd_in, 0);
    assert_M(file_data != MAP_FAILED, "Failed to map file to memory - %s", strerror(errno));

    // Decode file
    dxwifi_oti* oti      = file_data;
    uint32_t esi         = ntohl(oti->esi);
    uint32_t n           = ntohl(oti->n);
    uint32_t k           = ntohl(oti->k);
    uint32_t crc         = ntohl(oti->crc);
    uint32_t symbol_size = DXWIFI_FEC_SYMBOL_SIZE;

    log_fatal("esi=%d, n=%d, k=%d, symbol size=%d", esi, n, k);

    of_ldpc_parameters_t codec_params = {
        .nb_source_symbols      = k,
        .nb_repair_symbols      = n - k,
        .encoding_symbol_length = symbol_size,
        .prng_seed              = rand(),
        .N1                     = (n-k) > 10 ? 10 : (n-k)
    };
    of_session_t* openfec_session = NULL;
    of_status_t status = OF_STATUS_OK;

    status = of_create_codec_instance(&openfec_session, OF_CODEC_LDPC_STAIRCASE_STABLE, OF_DECODER, 2);
    assert_M(status == OF_STATUS_OK, "Failed to initialize OpenFEC session");

    status = of_set_fec_parameters(openfec_session, (of_parameters_t*) &codec_params);
    assert_M(status == OF_STATUS_OK, "Failed to set codec parameters");

    void* symbol_table[n];

    for (size_t i = 0; i < file_size; i += (symbol_size + sizeof(dxwifi_oti))) 
    {
        log_fatal("i=%d", i);
        oti = file_data + i;
        log_fatal("esi=%d", ntohl(oti->esi));

        of_decode_with_new_symbol(openfec_session, file_data + i + sizeof(dxwifi_oti), ntohl(oti->esi));
    }
    if(!of_is_decoding_complete(openfec_session)) {
        status = of_finish_decoding(openfec_session);
        if(status != OF_STATUS_OK) {
            log_fatal("Couldn't finish decoding");
            // Asssert here? log what we have?
        }
    }

    status = of_get_source_symbols_tab(openfec_session, symbol_table);
    if(status != OF_STATUS_OK) {
        log_fatal("Failed to get src symbols");
        // Asssert here? log what we have?
    }

    for(size_t esi = 0; esi < k; ++esi) {
        write(fd_out, symbol_table[esi], symbol_size);
    }
}


void decode_stream(cli_args* args) {
    assert_always("Unimplemented");
}