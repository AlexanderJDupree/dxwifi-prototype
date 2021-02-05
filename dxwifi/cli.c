/**
 * Command Line utility definitions
 */

#include <argp.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>

#include <libdxwifi/dxwifi.h>

#include <dxwifi/cli.h>

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
    dxwifi_transmitter* args = (dxwifi_transmitter*) state->input;

    switch (key)
    {
    case 'd':
        args->device = arg;
        break;

    case 'b':
        args->block_size = atoi(arg);
        if( args->block_size < DXWIFI_BLOCK_SIZE_MIN || args->block_size > DXWIFI_BLOCK_SIZE_MAX) {
            argp_error(
                state,
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
        args->rtap_rate = atoi(arg); // TODO error handling
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
        if(state->arg_num >= 1 || (args->fd = open(arg, O_RDONLY)) < 0) {
            argp_error(state, "Failed to open file for reading: %s\n", arg);
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

int parse_args(int argc, char** argv, dxwifi_transmitter* out) {

    return argp_parse(&argparser, argc, argv, 0, 0, out);

}
