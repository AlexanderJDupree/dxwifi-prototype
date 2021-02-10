/**
 *  Entry point for DxWifi packet capture
 */

#include <argp.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

#include <libdxwifi/dxwifi.h>
#include <libdxwifi/receiver.h>
#include <libdxwifi/details/logging.h>


typedef struct {
    int file;
    int append;
    int verbosity;
    dxwifi_receiver rx;
} cli_args;


dxwifi_receiver* receiver = NULL;


void sigint_handler(int signum);
int parse_args(int argc, char** argv, cli_args* out);
void logger(enum dxwifi_log_level verbosity, const char* fmt, va_list args);


int main(int argc, char** argv) {

    cli_args args = {
        .file                       = STDOUT_FILENO,
        .append                     = 0,
        .verbosity                  = DXWIFI_LOG_OFF,
        .rx = {
            .device                 = "mon0",
            .dispatch_count         = 5,
            .capture_timeout        = -1,
            .filter                 = "wlan addr2 aa:aa:aa:aa:aa:aa",
            .optimize               = true,
            .snaplen                = SNAPLEN_MAX,
            .packet_buffer_timeout  = DXWIFI_DFLT_PACKET_BUFFER_TIMEOUT
        }
    };
    receiver = &args.rx;

    parse_args(argc, argv, &args);

    init_logging(args.verbosity, logger);

    init_receiver(receiver);

    signal(SIGINT, sigint_handler);

    receiver_activate_capture(receiver, args.file);

    close_receiver(receiver);

    close(args.file);

    exit(0);
}


void sigint_handler(int signum) {
    signal(SIGINT, SIG_IGN);
    receiver_stop_capture(receiver);
    signal(SIGINT, SIG_DFL);
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
    "Capture packets matching a BPF program and output the data to output-file or stdout";

static struct argp_option opts[] = {
    { "dev",            'd',    "<network device>",     0,  "The interface to listen for packets on, must be enabled in monitor mode",  DXWIFI_RX_GROUP },
    { "timeout",        't',    "<seconds>",            0,  "Length of time, in seconds, to wait for a packet (default: infinity)",     DXWIFI_RX_GROUP },
    { "dispatch-count", 'c',    "<number>",             0,  "Number of packets to process at a time",                                   DXWIFI_RX_GROUP },
    { "append",         'a',    0,                      0,  "Open file in append mode",                                                 DXWIFI_RX_GROUP },

    { 0, 0,  0,  0, "Packet Capture Settings (https://www.tcpdump.org/manpages/pcap.3pcap.html)",           PCAP_SETTINGS_GROUP },
    { "snaplen",        's',    "<bytes>",      OPTION_NO_USAGE,    "Snapshot length",                      PCAP_SETTINGS_GROUP },
    { "buffer-timeout", 'b',    "<ms>",         OPTION_NO_USAGE,    "Packet buffer timeout",                PCAP_SETTINGS_GROUP },
    { "filter",         'f',    "<string>",     OPTION_NO_USAGE,    "Berkely Packet Filter expression",     PCAP_SETTINGS_GROUP },
    { "no-optimize",    'o',    0,              OPTION_NO_USAGE,    "Do not optimize the BPF expression",   PCAP_SETTINGS_GROUP },

    { 0, 0,  0,  0, "Help options", CLI_GROUP_LAST },
    { "verbose",    'v',    0,                      0,  "Verbosity level", CLI_GROUP_LAST},

    {0} // Final zero field is required by argp
};

static error_t parse_opt(int key, char* arg, struct argp_state* state) {

    error_t status  = 0;
    cli_args* args  = (cli_args*) state->input;
    int open_flags  = O_WRONLY | O_CREAT | args->append;
    mode_t mode     = S_IRWXU  | S_IRWXO;

    switch (key)
    {
    case 'd':
        args->rx.device = arg;
        break;

    case 'a':
        args->append = O_APPEND;
        break;

    case 't':
        args->rx.capture_timeout = atoi(arg); // TODO error handling
        break;

    case 'c':
        args->rx.dispatch_count = atoi(arg); // TODO error handling
        break;

    case 'v':
        args->verbosity++;
        break;

    case 's':
        args->rx.snaplen = atoi(arg); // TODO error handling
        break;

    case 'b':
        args->rx.packet_buffer_timeout = atoi(arg); 
        break;

    case 'f':
        args->rx.filter = arg;
        break;

    case 'o':
        args->rx.optimize = false;
        break;

    case ARGP_KEY_ARG:
        if(state->arg_num >= 1 || (args->file = open(arg, open_flags, mode)) < 0) {
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

