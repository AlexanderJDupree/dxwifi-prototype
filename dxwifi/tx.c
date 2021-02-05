/**
 *  Entry point for DxWifi packet transmission
 */
#include <stdio.h>
#include <stdlib.h>


#include <dxwifi/cli.h>

#include <libdxwifi/dxwifi.h>

int main(int argc, char** argv) {

    int status = 0;
    dxwifi_transmitter transmitter = { 
        .fd             = DXWIFI_DFLT_FILE,
        .device         = DXWIFI_DFLT_DEVICE,
        .verbosity      = DXWIFI_DFLT_VERBOSITY,
        .block_size     = DXWIFI_DFLT_BLK_SIZE,
        .rtap_flags     = DXWIFI_TX_DFLT_RADIOTAP_FLAGS,
        .rtap_rate      = DXWIFI_TX_DFLT_RADIOTAP_RATE,
        .rtap_tx_flags  = DXWIFI_TX_DFLT_RADIOTAP_TX_FLAGS
    };

    parse_args(argc, argv, &transmitter);

    init_transmitter(&transmitter);

    status = dxwifi_transmit(&transmitter);

    // Teardown resources - should do some final logging too
    close_transmitter(&transmitter);
    exit(status);
}
