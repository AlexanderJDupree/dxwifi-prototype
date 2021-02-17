/**
 * Entry point for DxWiFi block encoder
 */

#include <argp.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <libdxwifi/dxwifi.h>
#include <libdxwifi/details/logging.h>
#include <openfec/src/lib_common/of_openfec_api.h>


typedef struct {
    int         file;
    int         verbosity;
    unsigned    block_size;
    float       code_rate;

} cli_args;


int main(int argc, char** argv) {
    cli_args args = {
        .file = STDIN_FILENO,
        .verbosity = DXWIFI_LOG_OFF,
        .block_size = 1024,
        .code_rate = 0.667
    };
    of_codec_id_t   codec_id;
    of_status_t     of_status;

    struct stat file_stats;

    if( fstat(args.file, &file_stats) != 0 ) {
        printf("Error fstat\n");
        exit(1);
    }

    uint8_t* buffer = NULL;
    buffer = mmap(NULL, file_stats.st_size, PROT_READ, MAP_SHARED, args.file, 0);

    if( !buffer ) {
        printf("mmpa didn't work");
    }

    printf("\n\n%s\n\n", (char*)buffer);


    int status = munmap(buffer, file_stats.st_size);


    exit(status);
}


/************************
 *  CLI
 ***********************/

/**
 * Command Line Parser definition
 * 
 * Current implementation uses argp, to add a new command line option you will 
 * need to modify three things:
 *      - Add the storage type to the 'cli_args' structure
 *      - Add an argp_option struct to the opts array 
 *      - Add a case to the switch block in parse_opt() 
 */


const char* argp_program_version     = DXWIFI_VERSION;
const char* argp_program_bug_address = "TODO@gmail.com";

static char args_doc[] = "input-file";

static char doc[] = 
    "Block encode a file";

static struct argp_option opts[] = {
    { "verbose",                'v',    0,                      0,  "Verbosity level", 0},

    {0} // Final zero field is required by argp
};

static error_t parse_opt(int key, char* arg, struct argp_state* state) {

    error_t status  = 0;
    cli_args* args  = (cli_args*) state->input;

    switch (key)
    {

    case 'v':
        args->verbosity++;
        break;

    case ARGP_KEY_ARG:
        if(state->arg_num >= 1 || (args->file = open(arg, O_RDONLY)) < 0) {
            argp_error(state, "Failed to open file: %s", arg);
            argp_usage(state);
        }
        break;
    
    default:
        status = ARGP_ERR_UNKNOWN;
        break;
    }
    return status;
}


static struct argp argparser = { 
    .options        = opts, 
    .parser         = parse_opt, 
    .args_doc       = args_doc, 
    .doc            = doc, 
    .children       = 0, 
    .help_filter    = 0,
    .argp_domain    = 0
};


int parse_args(int argc, char** argv, cli_args* out) {

    return argp_parse(&argparser, argc, argv, 0, 0, out);

}
