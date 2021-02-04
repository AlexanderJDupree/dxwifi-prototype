/**
 *  Entry point for DxWifi packet transmission
 */
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>

#include <dxwifi/cli.h>
#include <dxwifi/dxwifi.h>

void log_configuration(command_args* args);

int main(int argc, char** argv) {

    int status = 0;
    int fd = fileno(stdin);
    dxwifi_transmitter transmitter;

    command_args args = { 
        .device         = DXWIFI_DFLT_DEVICE,
        .input_file     = DXWIFI_DFLT_INPUT_FILE,
        .verbosity      = DXWIFI_DFLT_VERBOSITY,
	    .block_size     = DXWIFI_DFLT_BLK_SIZE,
        .rtap_flags     = DXWIFI_DFLT_RADIOTAP_FLAGS,
        .rtap_data_rate = DXWIFI_DFLT_RADIOTAP_RATE,
        .rtap_tx_flags  = DXWIFI_DFLT_RADIOTAP_TX_FLAGS
    };

    parse_args(argc, argv, &args);

    log_configuration(&args);

    if (args.input_file != NULL && (fd = open(args.input_file, O_RDONLY)) < 0) {
        fprintf(stderr, "Failed to open file for reading: %s\n", args.input_file);
        exit(1);
    }

    init_transmitter(&transmitter, args.device);

    status = transmit_file(&transmitter, fd, args.block_size);

    // Teardown resources - should do some final logging too
    close_transmitter(&transmitter);
    close(fd);
    exit(status);
}

/// TODO Add actual logging library and better error handling
void log_configuration(command_args* args) {
    if (args->verbosity > 0) {
        printf(
            "Verbosity:     %d\n"
            "Device:        %s\n"
            "Input:         %s\n"
            "Data Rate:     %dMbps\n"
            "RTAP flags:    0x%x\n"
            "RTAP Tx flags: 0x%x\n",
            args->verbosity,
            args->device,
            args->input_file,
            args->rtap_data_rate,
            args->rtap_flags,
            args->rtap_tx_flags
        );
    }
}
