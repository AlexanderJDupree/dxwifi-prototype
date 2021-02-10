#!/bin/bash
# Simple script to enable monitor mode on a given interface
# Defaults to enabling monitor mode on mon0 at Channel 3 and txpower of 20dBM
# Must be run with sudo

set -e

dev=${1:-mon0}

function cleanup()
{
    if [[ $? -ne 0 ]]; then
        echo -e "Usage: sudo ./startMonitor.sh [interface:mon0] [channel:3] [txpower:20]"
    else
        echo -e "Enabled $dev in monitor mode"
    fi
}

# keep track of the last executed command
trap 'last_command=$current_command; current_command=$BASH_COMMAND' DEBUG
# echo an error message before exiting
trap cleanup EXIT

ip link set $dev down
iw dev $dev set monitor fcsfail otherbss
ip link set $dev up
iw dev $dev set channel ${2:-3}
iw dev $dev info
exit 0
