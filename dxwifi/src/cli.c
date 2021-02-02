/**
 * Command Line utility definitions
 */

#include <argp.h>

#include <dxwifi/cli.h>
#include <dxwifi/dxwifi.h>

const char* argp_program_version = DXWIFI_VERSION;

const char* argp_program_bug_address = "TODO@gmail.com";

/* Description of the key arguments */
static char args_doc[] = "input-file";

/* Program documentation */
static char doc[] = 
    "Reads bytes from input file and inject them over a monitor mode enabled WiFi interface";

/* Available command line options */
static struct argp_option opts[] = {
    { "dev",        'd',    "<network device>",     0,  "The interface to inject packets onto, must be enabled in monitor mode (default: mon0)" },
    { "verbose",    'v',    0,                      0,  "Verbosity level (default: 0)"},
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

    case 'v':
        args->verbosity++;
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

static struct argp argparser = { opts, parse_opt, args_doc, doc, 0, 0, 0};

int parse_args(int argc, char** argv, command_args* out) {

    return argp_parse(&argparser, argc, argv, 0, 0, out);

}
