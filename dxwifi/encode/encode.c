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

#include <of_openfec_api.h>

#include <dxwifi/encode/cli.h>
#include <libdxwifi/details/utils.h>
#include <libdxwifi/details/assert.h>
#include <libdxwifi/details/logging.h>


void encode_file(cli_args* args);
void encode_stream(cli_args* args);

int main(int argc, char** argv) {
    cli_args args = {
        .file_in    = NULL,
        .file_out   = NULL,
        .blocksize  = 1024,
        .coderate   = 0.667,
        .verbosity  = DXWIFI_LOG_INFO,
        .quiet      = false
    };

    parse_args(argc, argv, &args);

    set_log_level(DXWIFI_LOG_ALL_MODULES, args.verbosity);

    if(args.file_in) {
        encode_file(&args);
    }
    else {
        encode_stream(&args);
    }

    exit(0);
}


void encode_file(cli_args* args) {

    of_status_t status = OF_STATUS_OK;

    of_session_t* openfec_session = NULL;

    off_t file_size = get_file_size(args->file_in);
    assert_M(file_size > 0, "Failed to get file size");

    uint32_t k =  ceil((float) file_size / (float) args->blocksize);
    uint32_t n = k / args->coderate;  
    of_ldpc_parameters_t codec_params = {
        .nb_source_symbols      = k,
        .nb_repair_symbols      = n - k,
        .encoding_symbol_length = args->blocksize,
        .prng_seed              =  rand(), // TODO no seed for rand()

        // N1 denotes the target number of "1s" per column in the left side of the parity check matrix.
        .N1                     = (n-k) > 7 ? 7 : (n-k) // TODO magic number
    };


    log_info("k=%d, n=%d N1=%d", k, n);

    status = of_create_codec_instance(&openfec_session, OF_CODEC_LDPC_STAIRCASE_STABLE, OF_ENCODER, 2);
    assert_M(status == OF_STATUS_OK, "Failed to initialize OpenFEC Session");

    status = of_set_fec_parameters(openfec_session, (of_parameters_t*) &codec_params);
    assert_M(status == OF_STATUS_OK, "Failed to set codec parameters");

    int fd = open(args->file_in, O_RDWR);
    assert_M(fd > 0, "Failed to open file: %s - %s", args->file_in, strerror(errno));

    // Allocate N blocks of memory
    uint8_t* symbols = calloc(n, args->blocksize);
    assert_M(symbols, "Calloc failure");

    // Read data into first K blocks
    ssize_t nbytes = read(fd, symbols, file_size);
    assert_M(nbytes == file_size, "Failed to read file");

    close(fd);

    // Setup symbol table
    void* symbol_table[n];
    for(int esi = 0; esi < n; ++esi) {
        symbol_table[esi] = symbols + (esi * args->blocksize);
    }

    // Build repair symbols
    for(int esi = k; esi < n; ++esi) {
        status = of_build_repair_symbol(openfec_session, symbol_table, esi);
        assert_M(status == OF_STATUS_OK, "Failed to build repair symbol. esi=%d", esi);
    }

    int open_flags  = O_WRONLY | O_CREAT;
    mode_t mode     = S_IRUSR  | S_IWUSR | S_IROTH | S_IWOTH; 
    fd = args->file_out ? open(args->file_out, open_flags, mode) : STDOUT_FILENO;
    assert_M(fd > 0, "Failed to open file: %s - %s", args->file_out, strerror(errno));

    nbytes = write(fd, symbols, n * args->blocksize);
    assert_M(nbytes == (n * args->blocksize), "Partial write occured: %d - %s", nbytes, strerror(errno));

    of_release_codec_instance(openfec_session);

}


void encode_stream(cli_args* args) {
    assert_always("Unimplemented");
}