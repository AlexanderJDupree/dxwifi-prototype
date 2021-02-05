/**
 *  Entry point for DxWifi packet transmission
 */


#include <argp.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>

#include <libdxwifi/dxwifi.h>
#include <libdxwifi/details/logging.h>


#define DXWIFI_TX_DFLT_FILE                     0
#define DXWIFI_TX_DFLT_DEVICE                   "mon0"
#define DXWIFI_TX_DFLT_INPUT_FILE               0
#define DXWIFI_TX_DFLT_BLK_SIZE                 256
#define DXWIFI_TX_DFLT_RADIOTAP_FLAGS           IEEE80211_RADIOTAP_F_FCS
#define DXWIFI_TX_DFLT_RADIOTAP_RATE            1
#define DXWIFI_TX_DFLT_RADIOTAP_TX_FLAGS        IEEE80211_RADIOTAP_F_TX_NOACK
#define DXWIFI_TX_DFLT_VERBOSITY                DXWIFI_LOG_OFF

typedef struct {
    int file;
    dxwifi_transmitter tx;
} cli_args;

void logger(enum dxwifi_log_level verbosity, const char* fmt, va_list args);
int parse_args(int argc, char** argv, cli_args* out);


int main(int argc, char** argv) {

    int status = 0;

    cli_args args = {
        .file               = DXWIFI_TX_DFLT_FILE,
        .tx = {
            .device         = DXWIFI_TX_DFLT_DEVICE,
            .verbosity      = DXWIFI_TX_DFLT_VERBOSITY,
            .block_size     = DXWIFI_TX_DFLT_BLK_SIZE,
            .rtap_flags     = DXWIFI_TX_DFLT_RADIOTAP_FLAGS,
            .rtap_rate      = DXWIFI_TX_DFLT_RADIOTAP_RATE,
            .rtap_tx_flags  = DXWIFI_TX_DFLT_RADIOTAP_TX_FLAGS
        }
    };
    dxwifi_transmitter* transmitter = &args.tx;


    parse_args(argc, argv, &args);

    init_logging(transmitter->verbosity, logger);

    init_transmitter(transmitter);

    status = transmit_file(transmitter, args.file);

    // Teardown resources - should do some final logging too
    close_transmitter(transmitter);
    close(args.file);
    exit(status);
}


void logger(enum dxwifi_log_level log_level, const char* fmt, va_list args) {
    // For now just log everything to stdout
    printf("[ %s ] : ", log_level_to_str(log_level));
    vprintf(fmt, args);
    printf("\n");
}


/************************
 *  CLI
 ***********************/

/**
 * Command Line Parser definition
 * 
 * Current implementation uses argp, to add a new command line option you will 
 * need to modify three things:
 *      - Add the storage type to the 'dxwifi_transmitter' structure
 *      - Add an argp_option struct to the opts array 
 *      - Add a case to the switch block in parse_opt() 
 */

#define DXWIFI_GROUP 0
#define RADIOTAP_FLAGS_GROUP        1000
#define RADIOTAP_RATE_GROUP         1500
#define RADIOTAP_TX_FLAGS_GROUP     2000
#define CLI_GROUP_LAST              RADIOTAP_TX_FLAGS_GROUP + 1

#define GET_KEY(x, group) (x + group)

const char* argp_program_version = DXWIFI_VERSION;

const char* argp_program_bug_address = "TODO@gmail.com";

/* Description of the key arguments */
static char args_doc[] = "input-file";

/* Program documentation */
static char doc[] = 
    "Reads bytes from input file and inject them over a monitor mode enabled WiFi interface";

/* Available command line options */
static struct argp_option opts[] = {

    { "dev",        'd',    "<network device>",     0,  "The interface to inject packets onto, must be enabled in monitor mode", DXWIFI_GROUP},
    { "blocksize",  'b',    "<blocksize>",          0,  "Size in bytes for each block read from file", DXWIFI_GROUP},

    { 0, 0,  0,  0, "Radiotap Header Configuration Options", RADIOTAP_FLAGS_GROUP },
    { 0, 0,  0,  0, "WARN: The following fields are driver dependent and/or may not be supported by DxWifi. Most of these fields may or not have any effect on packet injection", RADIOTAP_FLAGS_GROUP },
    { "cfp",            GET_KEY(IEEE80211_RADIOTAP_F_CFP,           RADIOTAP_FLAGS_GROUP),      0,  OPTION_NO_USAGE,  "Sent during CFP",                        RADIOTAP_FLAGS_GROUP },
    { "short-preamble", GET_KEY(IEEE80211_RADIOTAP_F_SHORTPRE,      RADIOTAP_FLAGS_GROUP),      0,  OPTION_NO_USAGE,  "Sent with short preamble",               RADIOTAP_FLAGS_GROUP },
    { "wep",            GET_KEY(IEEE80211_RADIOTAP_F_WEP,           RADIOTAP_FLAGS_GROUP),      0,  OPTION_NO_USAGE,  "Sent with WEP encryption",               RADIOTAP_FLAGS_GROUP },
    { "frag",           GET_KEY(IEEE80211_RADIOTAP_F_FRAG,          RADIOTAP_FLAGS_GROUP),      0,  OPTION_NO_USAGE,  "Sent with fragmentation",                RADIOTAP_FLAGS_GROUP },
    { "nofcs",          GET_KEY(IEEE80211_RADIOTAP_F_FCS,           RADIOTAP_FLAGS_GROUP),      0,  OPTION_NO_USAGE,  "Frame does not includes FCS",            RADIOTAP_FLAGS_GROUP },

    { "rate",           GET_KEY(IEEE80211_RADIOTAP_RATE,            RADIOTAP_RATE_GROUP),       0,  OPTION_NO_USAGE,  "Tx data rate (Mbps)",                    RADIOTAP_RATE_GROUP },

    { "ack",            GET_KEY(IEEE80211_RADIOTAP_F_TX_NOACK,      RADIOTAP_TX_FLAGS_GROUP),   0,  OPTION_NO_USAGE,  "Tx expects an ACK frame",                RADIOTAP_TX_FLAGS_GROUP },
    { "sequence",       GET_KEY(IEEE80211_RADIOTAP_F_TX_NOSEQNO,    RADIOTAP_TX_FLAGS_GROUP),   0,  OPTION_NO_USAGE,  "Tx includes preconfigured sequence id",  RADIOTAP_TX_FLAGS_GROUP },
    { "ordered",        GET_KEY(IEEE80211_RADIOTAP_F_TX_NOACK,      RADIOTAP_TX_FLAGS_GROUP),   0,  OPTION_NO_USAGE,  "Tx should not be reordered",             RADIOTAP_TX_FLAGS_GROUP },

    { 0, 0,  0,  0, "Help options", CLI_GROUP_LAST},
    { "verbose",    'v',    0,                      0,  "Verbosity level", CLI_GROUP_LAST},

    {0} // Final zero field is required by argp
}; 

static error_t parse_opt(int key, char* arg, struct argp_state *state) {

    error_t status = 0;
    cli_args* args = (cli_args*) state->input;

    switch (key)
    {
    case 'd':
        args->tx.device = arg;
        break;

    case 'b':
        args->tx.block_size = atoi(arg);
        if( args->tx.block_size < DXWIFI_BLOCK_SIZE_MIN || args->tx.block_size > DXWIFI_BLOCK_SIZE_MAX) {
            argp_error(
                state,
                "blocksize of `%ld` not in range(%d,%d)\n", 
                args->tx.block_size, 
                DXWIFI_BLOCK_SIZE_MIN,
                DXWIFI_BLOCK_SIZE_MAX
                );
            argp_usage(state);
        }
        break;

    case 'v':
        args->tx.verbosity++;
        break;

    case GET_KEY(IEEE80211_RADIOTAP_F_CFP, RADIOTAP_FLAGS_GROUP):
        args->tx.rtap_flags |= IEEE80211_RADIOTAP_F_CFP;
        break;

    case GET_KEY(IEEE80211_RADIOTAP_F_SHORTPRE, RADIOTAP_FLAGS_GROUP):
        args->tx.rtap_flags |= IEEE80211_RADIOTAP_F_SHORTPRE;
        break;

    case GET_KEY(IEEE80211_RADIOTAP_F_WEP, RADIOTAP_FLAGS_GROUP):
        args->tx.rtap_flags |= IEEE80211_RADIOTAP_F_WEP;
        break;

    case GET_KEY(IEEE80211_RADIOTAP_F_FRAG, RADIOTAP_FLAGS_GROUP):
        args->tx.rtap_flags |= IEEE80211_RADIOTAP_F_FRAG;
        break;

    // Clear bit since default is on
    case GET_KEY(IEEE80211_RADIOTAP_F_FCS, RADIOTAP_FLAGS_GROUP):
        args->tx.rtap_flags &= ~(IEEE80211_RADIOTAP_F_FCS);
        break;

    case GET_KEY(IEEE80211_RADIOTAP_RATE, RADIOTAP_RATE_GROUP):
        args->tx.rtap_rate = atoi(arg); // TODO error handling
        break;

    // Clear bit since default is on
    case GET_KEY(IEEE80211_RADIOTAP_F_TX_NOACK, RADIOTAP_TX_FLAGS_GROUP):
        args->tx.rtap_tx_flags &= ~(IEEE80211_RADIOTAP_F_TX_NOACK);
        break;

    case GET_KEY(IEEE80211_RADIOTAP_F_TX_NOSEQNO, RADIOTAP_TX_FLAGS_GROUP):
        args->tx.rtap_tx_flags |= IEEE80211_RADIOTAP_F_TX_NOSEQNO;
        break;

    case GET_KEY(IEEE80211_RADIOTAP_F_TX_ORDER, RADIOTAP_TX_FLAGS_GROUP):
        args->tx.rtap_tx_flags |= IEEE80211_RADIOTAP_F_TX_ORDER;
        break;

    case ARGP_KEY_ARG:
        if(state->arg_num >= 1 || (args->file = open(arg, O_RDONLY)) < 0) {
            argp_error(state, "Failed to open file: %s", arg);
            argp_usage(state);
        }
        break;

    default:
        status = ARGP_ERR_UNKNOWN;
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