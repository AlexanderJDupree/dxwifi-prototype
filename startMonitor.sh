#!/bin/bash
# Simple script to enable monitor mode on a given interface
# Defaults to enabling monitor mode on mon0 at Channel 3 and txpower of 20dBM
# Must be run with sudo
#
# TODO add verbose flag, make script more robust

set -e

dev=${1:-mon0}

# keep track of the last executed command
trap 'last_command=$current_command; current_command=$BASH_COMMAND' DEBUG
# echo an error message before exiting
trap 'echo "\"${last_command}\" command filed with exit code $?."' EXIT

ip link set $dev down
iw dev $dev set monitor fcsfail otherbss
ip link set $dev up
iw dev $dev set channel ${2:-3}
echo -e "Enabled $dev in monitor mode"
exit 0
