/**
 *  Entry point for DxWifi packet capture
 */

#include <argp.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>

#include <libdxwifi/dxwifi.h>
#include <libdxwifi/receiver.h>
#include <libdxwifi/details/logging.h>


#define DXWIFI_RX_DFLT_FILE         STDOUT_FILENO
#define DXWIFI_RX_DFLT_VERBOSITY    DXWIFI_LOG_OFF
#define DXWIFI_RX_DFLT_DEVICE       "mon0"
#define DXWIFI_RX_DFLT_FILTER       "wlan addr2 aa:aa:aa:aa:aa:aa"
#define DXWIFI_RX_DFLT_SNAPLEN      SNAPLEN_MAX
#define DXWIFI_RX_DFLT_TIMEOUT      DXWIFI_DFLT_PACKET_BUFFER_TIMEOUT


typedef struct {
    int file;
    int verbosity;
    dxwifi_receiver rx;
} cli_args;


int parse_args(int argc, char** argv, cli_args* out);
void logger(enum dxwifi_log_level verbosity, const char* fmt, va_list args);


int main(int argc, char** argv) {

    int status  = 0;

    cli_args args = {
        .file                       = DXWIFI_RX_DFLT_FILE,
        .verbosity                  = DXWIFI_RX_DFLT_VERBOSITY,
        .rx = {
            .device                 = DXWIFI_RX_DFLT_DEVICE,
            .filter                 = DXWIFI_RX_DFLT_FILTER,
            .optimize               = true,
            .snaplen                = DXWIFI_RX_DFLT_SNAPLEN,
            .packet_buffer_timeout  = DXWIFI_RX_DFLT_TIMEOUT
        }
    };
    dxwifi_receiver* receiver = &args.rx;

    parse_args(argc, argv, &args);

    init_logging(args.verbosity, logger);

    init_receiver(receiver);

    receiver_activate_capture(receiver, args.file);

    close_receiver(receiver);

    return status;
}


void logger(enum dxwifi_log_level log_level, const char* fmt, va_list args) {
    // For now just log everything to stdout
    fprintf(stderr, "[ %s ] : ", log_level_to_str(log_level));
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
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

#define DXWIFI_RX_GROUP     0
#define PCAP_SETTINGS_GROUP 100
#define CLI_GROUP_LAST      PCAP_SETTINGS_GROUP + 1


const char* argp_program_version     = DXWIFI_VERSION;
const char* argp_program_bug_address = "TODO@gmail.com";

static char args_doc[] = "output-file";

static char doc[] = 
    "Capture packets matching a BPF program and output the data to output-file";

static struct argp_option opts[] = {
    { "dev",        'd',    "<network device>",     0,  "The interface to listen for packets on, must be enabled in monitor mode", DXWIFI_RX_GROUP },

    { 0, 0,  0,  0, "Packet Capture Settings (https://www.tcpdump.org/manpages/pcap.3pcap.html)",           PCAP_SETTINGS_GROUP },
    { "snaplen",        's',    "<bytes>",      OPTION_NO_USAGE,    "Snapshot length",                      PCAP_SETTINGS_GROUP },
    { "timeout",        't',    "<ms>",         OPTION_NO_USAGE,    "Packet buffer timeout",                PCAP_SETTINGS_GROUP },
    { "filter",         'f',    "<string>",     OPTION_NO_USAGE,    "Berkely Packet Filter expression",     PCAP_SETTINGS_GROUP },
    { "no-optimize",    'o',    0,              OPTION_NO_USAGE,    "Do not optimize the BPF expression",   PCAP_SETTINGS_GROUP },

    { 0, 0,  0,  0, "Help options", CLI_GROUP_LAST },
    { "verbose",    'v',    0,                      0,  "Verbosity level", CLI_GROUP_LAST},

    {0} // Final zero field is required by argp
};

static error_t parse_opt(int key, char* arg, struct argp_state* state) {

    error_t status = 0;
    cli_args* args = (cli_args*) state->input;

    switch (key)
    {
    case 'd':
        args->rx.device = arg;
        break;

    case 'v':
        args->verbosity++;
        break;

    case 's':
        args->rx.snaplen = atoi(arg); // TODO error handling
        break;

    case 't':
        args->rx.packet_buffer_timeout = atoi(arg); 
        break;

    case 'f':
        args->rx.filter = arg;
        break;

    case 'o':
        args->rx.optimize = false;
        break;

    case ARGP_KEY_ARG:
        if(state->arg_num >= 1 || (args->file = open(arg, O_WRONLY | O_CREAT)) < 0) {
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

