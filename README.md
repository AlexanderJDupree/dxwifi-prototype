# DxWiFi Software Prototype

## Introduction
 
Personal prototype for the DxWiFi component of the 
[OreSat Live](https://www.oresat.org/satellites/oresat) project

## DxWiFi

OreSat Live is tasked with the mission of streaming images of Oregon from
low earth orbit (~400km) to DIY Handheld ground stations built by high-school 
students with cheap, COTS WiFi adapters. The 802.11 WiFi standard was designed 
for reliability over a Local Area Network in mind, and has some significant 
shortcomings when it comes to long range transmission. 

DxWiFi attempts to ssurmount these shortcomings by leveraging packet injection 
and monitoring. Typically used by security researchers (or nefarious hackers), 
packet injection and monitoring has seen extensive use in the testing (and breaking)
of 802.11 protocols. With packet injection we can send custom raw data frames 
over the air without the need of a network connection or an expectation of a 
positive ACK from the receiver. Likewise, receivers enabled in monitor mode can
"promiscuously" scan and capture any packet regardless of destination. Combining 
injection and monitoring enables DxWiFi to achieve true unidirectional communication 
without the need for control packets or ACKs. 

## Prerequisites

- 2 linux capable devices (Raspberry PI, Beaglebone Black, Your laptop etc.)
- 2 WiFi adapters capable of monitor mode (Only tested on Atheros AR9271 chipset)
- [CMake](https://cmake.org/)
- [libpcap](https://www.tcpdump.org/)

## Building DxWifi

DxWiFi uses [cmake](https://cmake.org/) to generate the correct build files. If 
you need to generate a build system for a specific platform then I suggest 
checking out the docs. For basic GNU Makefiles the following should get you going:

```bash
mkdir build
cd build && cmake ..
make 
```

Alternatively, to make multiple out-of-source builds you can do the following 
with any of the default configurations: *Debug, Release, RelWithDebInfo, MinSizeRel*
```
cmake -S . -B build/Debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build/Debug
```

The executables are located in `bin` and libraries in `lib`. You can also build a 
specific target with 

```
cmake --build --target dxwifi
```

**Note**: If you're using VSCode I *hightly* recommend just using the 
[CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)
extension. 

## Using DxWiFi

This current protoype has only 2 programs, `tx` and `rx` that simply attempt to
stream and receive arbitrary data. There is no Forward Error Correction or 
packet erasure strategies in place yet. Both programs are highly configurable 
and expose a sane interface for modifying all the different injection/monitoring
parameters. Here's a few usage examples:


On the receiver
```
sudo ./startMonitor.sh mon0
sudo ./rx --dev mon0 copy.md
```

On the transmitter 
```
sudo ./startMonitor.sh mon0
sudo ./tx --dev mon0 README.md
```

Congratulations you just copied this readme to the receiving device! 
(*Note* change `mon0` to the name of *your* network device)

Both the receiver support a variety of different options. You can get a full list
of options with the `--help` option. 

Here's an example where we set the receiver to open test.cap in append mode, timeout
after 10 seconds without receiving a packet, log everything, and only capture packets whose addressed to 11:22:33:44:55:66
```
sudo ./rx --dev mon0 -vvvvvv --append --timeout 10 --filter 'wlan addr1 11:22:33:44:55:66' test.cap
```

And for the transmitter we set it to transmit everything being read from stdin by 
omitting the file argument, set the blocksize to 200 bytes, address the packets to 
11:22:33:44:55:66, log everything, and specify that the packets should be strictly ordered
```
sudo ./tx --dev mon0 --blocksize 200 --addr1 11:22:33:44:55:66 --ordered -vvvvvv
``` 
