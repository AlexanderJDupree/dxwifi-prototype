/**
 * Command Line Parser definition
 * 
 * Current implementation uses argp, to add a new command line option you will 
 * need to modify three things:
 *      - Add the storage type to the 'dxwifi_transmitter' structure
 *      - Add an argp_option struct to the opts array in cli.c
 *      - Add a case to the switch block in the parse_opt() function in cli.c
 *      - If a default value is needed then modify the args struct in main
 */

#ifndef DXWIFI_CLI_H
#define DXWIFI_CLI_H

#include <libdxwifi/dxwifi.h>

int parse_args(int argc, char** argv, dxwifi_transmitter* out);

#endif // DXWIFI_CLI_H
