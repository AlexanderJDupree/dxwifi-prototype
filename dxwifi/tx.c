/**
 *  Entry point for DxWifi packet transmission
 */


#include <argp.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#include <libdxwifi/dxwifi.h>
#include <libdxwifi/transmitter.h>
#include <libdxwifi/details/logging.h>


typedef struct {
    int file;
    int verbosity;
    dxwifi_transmitter tx;
} cli_args;


dxwifi_transmitter* transmitter = NULL;


void sigint_handler(int signum);
int parse_args(int argc, char** argv, cli_args* out);
void logger(enum dxwifi_log_level verbosity, const char* fmt, va_list args);


int main(int argc, char** argv) {

    int status = 0;

    cli_args args = {
        .file                   = STDIN_FILENO,
        .verbosity              = DXWIFI_LOG_OFF,
        .tx = {
            .device             = "mon0",
            .block_size         = 512,
            .transmit_timeout   = -1,
            .rtap_flags         = IEEE80211_RADIOTAP_F_FCS,
            .rtap_rate          = 1,
            .rtap_tx_flags      = IEEE80211_RADIOTAP_F_TX_NOACK,

            // Frame control isn't hooked up to the CLI yet
            .fctl  = {
                .protocol_version   = IEEE80211_PROTOCOL_VERSION,
                .type               = IEEE80211_FTYPE_DATA,
                .stype              = { IEEE80211_STYPE_DATA },
                .to_ds              = false,
                .from_ds            = true,
                .more_frag          = false,
                .retry              = false,
                .power_mgmt         = false,
                .more_data          = true, 
                .wep                = false,
                .order              = false
            },

            .addr1 = { 0x05, 0x05, 0x05, 0x05, 0x05, 0x05 },
            .addr2 = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA },
            .addr3 = { 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00 }
        }
    };
    transmitter = &args.tx;

    parse_args(argc, argv, &args);

    init_logging(args.verbosity, logger);

    init_transmitter(transmitter);

    signal(SIGINT, sigint_handler);

    start_transmission(transmitter, args.file);

    close_transmitter(transmitter);

    close(args.file);

    exit(status);
}


void sigint_handler(int signum) {
    signal(SIGINT, SIG_IGN);
    stop_transmission(transmitter);
    signal(SIGINT, SIG_DFL);
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
 * 
 * Note: Groups are needed to define a unique key for each option as well as
 *  format the help message.
 * 
 */

#define DXWIFI_TX_GROUP             0
#define MAC_ADDRESS_GROUP           500
#define RADIOTAP_FLAGS_GROUP        1500
#define RADIOTAP_RATE_GROUP         2000
#define RADIOTAP_TX_FLAGS_GROUP     2500
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

    { "dev",        'd',    "<network device>",     0,  "The interface to inject packets onto, must be enabled in monitor mode",    DXWIFI_TX_GROUP },
    { "blocksize",  'b',    "<blocksize>",          0,  "Size in bytes for each block read from file",                              DXWIFI_TX_GROUP },
    { "timeout",    't',    "<seconds>",            0,  "Length of time in seconds to wait for an available read from file",        DXWIFI_TX_GROUP },

    { 0, 0,  0,  0, "IEEE80211 MAC Header Configuration Options", MAC_ADDRESS_GROUP },
    { "addr1",   GET_KEY(1, MAC_ADDRESS_GROUP), "<macaddr>", OPTION_NO_USAGE, "Default (05:05:05:05:05:05)" },
    { "addr2",   GET_KEY(2, MAC_ADDRESS_GROUP), "<macaddr>", OPTION_NO_USAGE, "Default (AA:AA:AA:AA:AA:AA)" },
    { "addr3",   GET_KEY(3, MAC_ADDRESS_GROUP), "<macaddr>", OPTION_NO_USAGE, "Default (FF:00:FF:00:FF:00)" },

    { 0, 0,  0,  0, "Radiotap Header Configuration Options (WARN: The following fields are driver dependent and/or may not be supported by DxWifi",             RADIOTAP_FLAGS_GROUP },
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

static bool parse_mac_address(const char* arg, uint8_t* mac) {
    return sscanf(arg, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", mac, mac + 1, mac + 2, mac + 3, mac + 4, mac + 5) == 6;
}

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

    case 't':
        args->tx.transmit_timeout = atoi(arg);
        break;

    case 'v':
        args->verbosity++;
        break;

    case GET_KEY(1, MAC_ADDRESS_GROUP):
        if( !parse_mac_address(arg, args->tx.addr1))
        {
            argp_error(state, "Mac address must be 6 octets in hexadecimal format delimited by a ':'");
            argp_usage(state);
        }
        break;

    case GET_KEY(2, MAC_ADDRESS_GROUP):
        if( !parse_mac_address(arg, args->tx.addr2) )
        {
            argp_error(state, "Mac address must be 6 octets in hexadecimal format delimited by a ':'");
            argp_usage(state);
        }
        break;

    case GET_KEY(3, MAC_ADDRESS_GROUP):
        if( !parse_mac_address(arg, args->tx.addr3) )
        {
            argp_error(state, "Mac address must be 6 octets in hexadecimal format delimited by a ':'");
            argp_usage(state);
        }
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