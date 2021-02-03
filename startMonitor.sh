#!/bin/bash
# Simple script to enable monitor mode on a given interface
# Defaults to enabling monitor mode on mon0 at Channel 3 and txpower of 20dBM
# Must be run with sudo
#
# TODO add verbose flag, make script more robust

dev=${1:-mon0}

ip link set $dev down
if [ $? -eq 0 ]; then
	iw dev $dev set monitor fcsfail otherbss
	ip link set $dev up
	iw dev $dev set channel ${2:-3}
	echo -e "Enabled $dev in monitor mode"
	exit 0
else
	echo -e "Error, could not find device $dev"
	echo -e "Usage: sudo startMonitor.sh [device:mon0] [channel:3]"
	exit 1
fi

