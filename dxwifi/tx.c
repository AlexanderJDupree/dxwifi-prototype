/**
 *  Entry point for DxWifi packet transmission
 */
#include <stdio.h>
#include <stdlib.h>

#include <dxwifi/cli.h>
#include <dxwifi/dxwifi.h>

void log_and_exit(int status);
void log_configuration(command_args* args);
int transmit_file(dxwifi_transmitter* transmit, FILE* fd);

int main(int argc, char** argv) {

    FILE *fd = stdin; // Defaults to reading from stdin
    int status = 0;
    dxwifi_transmitter transmitter;

    command_args args = { 
        .verbosity  = 0,
        .device     = DXWIFI_DFLT_DEVICE,
        .input_file = NULL
    };

    parse_args(argc, argv, &args);

    log_configuration(&args);

    if (args.input_file != NULL && (fd = fopen(args.input_file, "rb")) == NULL) {
        log_and_exit(1);
    }

    init_transmitter(&transmitter, args.device);

    status = transmit_file(&transmitter, fd);

    // Teardown resources - should do some final logging too
    close_transmitter(&transmitter);
    fclose(fd);
    return status;
}

/// TODO Add actual logging library and better error handling
void log_configuration(command_args* args) {
    if (args->verbosity > 0) {
        printf(
            "Verbosity:     %d\n"
            "Device:        %s\n"
            "Input:         %s\n",
            args->verbosity,
            args->device,
            args->input_file
        );
    }
}
void log_and_exit(int status) {
    fprintf(stderr, "%d: TODO Add error string lookup table for status codes!\n", status);
    exit(1);
}

int transmit_file(dxwifi_transmitter* transmit, FILE* fd) {
    // Loop
        // Read block size chunk from file
        // Generate packet headers and interleave data with FEC codes
        // inject packet
    return 0;
}
