/**
 * DxWifi project includes, definitions, and types
 */


#ifndef LIBDXWIFI_H
#define LIBDXWIFI_H

#include <libdxwifi/details/ieee80211.h>

/************************
 *  Versioning
 ***********************/

#define DXWIFI_VERSION_MAJOR    0
#define DXWIFI_VERSION_MINOR    1
#define DXWIFI_VERSION_PATCH    0
#define DXWIFI_VERSION_RELEASE  "alpha"

#define STRINGIFY(x)  STRINGIFY_(x)
#define STRINGIFY_(x) #x

#define DXWIFI_VERSION      STRINGIFY(DXWIFI_VERSION_MAJOR) "." \
                            STRINGIFY(DXWIFI_VERSION_MINOR) "." \
                            STRINGIFY(DXWIFI_VERSION_PATCH) "-" \
                            DXWIFI_VERSION_RELEASE


/************************
 *  Constants
 ***********************/

// https://www.tcpdump.org/manpages/pcap.3pcap.html
#define SNAPLEN_MAX 65535

#define DXWIFI_DFLT_PACKET_BUFFER_TIMEOUT 20

#define DXWIFI_BLOCK_SIZE_MIN 0
#define DXWIFI_BLOCK_SIZE_MAX 1400


#endif // LIBDXWIFI_H
