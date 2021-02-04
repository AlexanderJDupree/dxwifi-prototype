/**
 * Command Line utility definitions
 */

#include <argp.h>
#include <stdlib.h>

#include <dxwifi/cli.h>
#include <dxwifi/dxwifi.h>

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

    // DxWifi configuration fields
    { "dev",        'd',    "<network device>",     0,  "The interface to inject packets onto, must be enabled in monitor mode", 0},
    { "blocksize",  'b',    "<blocksize>",          0,  "Size in bytes for each block read from file", 0},
    { "verbose",    'v',    0,                      0,  "Verbosity level", CLI_GROUP_LAST},

    // Radiotap configuration fields NOTE: Most of these fields are driver dependent or not supported by DxWifi, they're included here because things change
    { "cfp",            GET_KEY(IEEE80211_RADIOTAP_F_CFP,           RADIOTAP_FLAGS_GROUP),      0,  OPTION_NO_USAGE,  "Sent during CFP",                        RADIOTAP_FLAGS_GROUP },
    { "short-preamble", GET_KEY(IEEE80211_RADIOTAP_F_SHORTPRE,      RADIOTAP_FLAGS_GROUP),      0,  OPTION_NO_USAGE,  "Sent with short preamble",               RADIOTAP_FLAGS_GROUP },
    { "wep",            GET_KEY(IEEE80211_RADIOTAP_F_WEP,           RADIOTAP_FLAGS_GROUP),      0,  OPTION_NO_USAGE,  "Sent with WEP encryption",               RADIOTAP_FLAGS_GROUP },
    { "frag",           GET_KEY(IEEE80211_RADIOTAP_F_FRAG,          RADIOTAP_FLAGS_GROUP),      0,  OPTION_NO_USAGE,  "Sent with fragmentation",                RADIOTAP_FLAGS_GROUP },
    { "nofcs",          GET_KEY(IEEE80211_RADIOTAP_F_FCS,           RADIOTAP_FLAGS_GROUP),      0,  OPTION_NO_USAGE,  "Frame does not includes FCS",            RADIOTAP_FLAGS_GROUP },

    { "rate",           GET_KEY(IEEE80211_RADIOTAP_RATE,            RADIOTAP_RATE_GROUP),       0,  OPTION_NO_USAGE,  "Tx data rate (Mbps)",                    RADIOTAP_RATE_GROUP },

    { "ack",            GET_KEY(IEEE80211_RADIOTAP_F_TX_NOACK,      RADIOTAP_TX_FLAGS_GROUP),   0,  OPTION_NO_USAGE,  "Tx expects an ACK frame",                RADIOTAP_TX_FLAGS_GROUP },
    { "sequence",       GET_KEY(IEEE80211_RADIOTAP_F_TX_NOSEQNO,    RADIOTAP_TX_FLAGS_GROUP),   0,  OPTION_NO_USAGE,  "Tx includes preconfigured sequence id",  RADIOTAP_TX_FLAGS_GROUP },
    { "ordered",        GET_KEY(IEEE80211_RADIOTAP_F_TX_NOACK,      RADIOTAP_TX_FLAGS_GROUP),   0,  OPTION_NO_USAGE,  "Tx should not be reordered",             RADIOTAP_TX_FLAGS_GROUP },

    {0} // Final zero field is required by argp
}; 

static error_t parse_opt(int key, char* arg, struct argp_state *state) {

    error_t status = 0;
    command_args* args = (command_args*) state->input;

    switch (key)
    {
    case 'd':
        args->device = arg;
        break;

    case 'b':
        args->block_size = atoi(arg);
        if( args->block_size < DXWIFI_BLOCK_SIZE_MIN || args->block_size > DXWIFI_BLOCK_SIZE_MAX) {
            fprintf(stderr, 
                "blocksize of `%ld` not in range(%d,%d)\n", 
                args->block_size, 
                DXWIFI_BLOCK_SIZE_MIN,
                DXWIFI_BLOCK_SIZE_MAX
                );
            argp_usage(state);
        }
        break;

    case 'v':
        args->verbosity++;
        break;

    case GET_KEY(IEEE80211_RADIOTAP_F_CFP, RADIOTAP_FLAGS_GROUP):
        args->rtap_flags |= IEEE80211_RADIOTAP_F_CFP;
        break;

    case GET_KEY(IEEE80211_RADIOTAP_F_SHORTPRE, RADIOTAP_FLAGS_GROUP):
        args->rtap_flags |= IEEE80211_RADIOTAP_F_SHORTPRE;
        break;

    case GET_KEY(IEEE80211_RADIOTAP_F_WEP, RADIOTAP_FLAGS_GROUP):
        args->rtap_flags |= IEEE80211_RADIOTAP_F_WEP;
        break;

    case GET_KEY(IEEE80211_RADIOTAP_F_FRAG, RADIOTAP_FLAGS_GROUP):
        args->rtap_flags |= IEEE80211_RADIOTAP_F_FRAG;
        break;

    // Clear bit since default is on
    case GET_KEY(IEEE80211_RADIOTAP_F_FCS, RADIOTAP_FLAGS_GROUP):
        args->rtap_flags &= ~(IEEE80211_RADIOTAP_F_FCS);
        break;

    case GET_KEY(IEEE80211_RADIOTAP_RATE, RADIOTAP_RATE_GROUP):
        args->rtap_data_rate = atoi(arg); // TODO error handling
        break;

    // Clear bit since default is on
    case GET_KEY(IEEE80211_RADIOTAP_F_TX_NOACK, RADIOTAP_TX_FLAGS_GROUP):
        args->rtap_tx_flags &= ~(IEEE80211_RADIOTAP_F_TX_NOACK);
        break;

    case GET_KEY(IEEE80211_RADIOTAP_F_TX_NOSEQNO, RADIOTAP_TX_FLAGS_GROUP):
        args->rtap_tx_flags |= IEEE80211_RADIOTAP_F_TX_NOSEQNO;
        break;

    case GET_KEY(IEEE80211_RADIOTAP_F_TX_ORDER, RADIOTAP_TX_FLAGS_GROUP):
        args->rtap_tx_flags |= IEEE80211_RADIOTAP_F_TX_ORDER;
        break;

    case ARGP_KEY_ARG:
        if(state->arg_num >= 1) {
            argp_usage(state);
        } else {
            args->input_file = arg;
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

int parse_args(int argc, char** argv, command_args* out) {

    return argp_parse(&argparser, argc, argv, 0, 0, out);

}
