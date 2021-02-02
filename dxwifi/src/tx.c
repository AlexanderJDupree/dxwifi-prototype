/**
 *  Entry point for DxWifi packet transmission
 */

#include <stdio.h>
#include <stdbool.h>

#include <pcap.h>

#include <dxwifi/cli.h>
#include <dxwifi/dxwifi.h>

int main(int argc, char** argv) {

    int return_code = 0;

    pcap_t* pcap_handle = NULL;

    char error_buffer[PCAP_ERRBUF_SIZE];
    

    command_args args = { 
        .verbosity  = 0,
        .device     = "mon0",
        .input_file = NULL
    };

    return_code = parse_args(argc, argv, &args);
    if( return_code != 0) {
        return 1;
    }

    // Configure interface

    // Open input file (or just read from STDIN)

    // Loop
        // Read block size chuck from file
        // Generate packet headers and interleave data with FEC codes
        // inject packet

    // Teardown resources

}