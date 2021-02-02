/**
 * Command Line Parser definition
 * 
 * Current implementation uses argp, to add a new command line option you will 
 * need to modify three things:
 *      - Add the storage type to the 'command_args' structure
 *      - Add an argp_option struct to the opts array in cli.c
 *      - Add a case to the switch block in the parse_opt() function in cli.c
 */

#ifndef DXWIFI_CLI_H
#define DXWIFI_CLI_H

typedef struct {
    int verbosity;
    const char* device;
    const char* input_file;
} command_args;

int parse_args(int argc, char** argv, command_args* out);

#endif // DXWIFI_CLI_H