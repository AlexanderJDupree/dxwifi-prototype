/**
 * Command Line Parser definition
 * 
 * Current implementation uses argp, to add a new command line option you will 
 * need to modify three things:
 *      - Add the storage type to the 'command_args' structure
 *      - Add an argp_option struct to the opts array in cli.c
 *      - Add a case to the switch block in the parse_opt() function in cli.c
 *      - If a default value is needed then modify the args struct in main
 */

#ifndef DXWIFI_CLI_H
#define DXWIFI_CLI_H

#include <stdint.h>

typedef struct {
    const char* device;
    const char* input_file;
    int         verbosity;
    size_t      block_size;
    uint8_t     rtap_flags;
    uint8_t     rtap_data_rate;
    uint16_t    rtap_tx_flags;
} command_args;

int parse_args(int argc, char** argv, command_args* out);

#endif // DXWIFI_CLI_H
