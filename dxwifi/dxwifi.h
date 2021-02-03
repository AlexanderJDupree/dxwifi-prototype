/**
 * DxWifi project includes, definitions, and types
 */


#ifndef DXWIFI_H
#define DXWIFI_H

#include <stdint.h>

#include <pcap.h>
#include <radiotap/radiotap.h>

/**
 * Versioning
 */

#define DXWIFI_VERSION_MAJOR    0
#define DXWIFI_VERSION_MINOR    1
#define DXWIFI_VERSION_PATCH    0
#define DXWIFI_VERSION_RELEASE  "alpha"

#define STRINGIFY(x) STRINGIFY_(x)
#define STRINGIFY_(x) #x

#define DXWIFI_VERSION      STRINGIFY(DXWIFI_VERSION_MAJOR) "." \
                            STRINGIFY(DXWIFI_VERSION_MINOR) "." \
                            STRINGIFY(DXWIFI_VERSION_PATCH) "-" \
                            DXWIFI_VERSION_RELEASE

/**
 *  Default Arguments
 */
#define DXWIFI_DFLT_DEVICE 			"mon0"
#define DXWIFI_DFLT_BLK_SIZE 			1400
#define DXWIFI_DFLT_PACKET_BUFFER_TIMEOUT 	20

/**
 *  Limits
 */

// https://networkengineering.stackexchange.com/questions/32970/what-is-the-802-11-mtu
#define MAX_PACKET_LENGTH 2304

// https://www.tcpdump.org/manpages/pcap.3pcap.html
#define SNAPLEN_MAX 65535

// TODO this were defined arbitrarily. Needs review
#define DXWIFI_BLOCK_SIZE_MIN 0
#define DXWIFI_BLOCK_SIZE_MAX 1500

#define DXWIFI_HEADER_SIZE (sizeof(ieee80211_radiotap_hdr) + sizeof(ieee80211_hdr))

// Length of MAC address in bytes
#define IEEE80211_MAC_ADDR_LEN 6

/**
 *  Data structures
 */

// Typedef for abbreviation
typedef struct ieee80211_radiotap_header ieee80211_radiotap_hdr;

/* Defined in github.com/torvalds/linux/include/linux/ieee80211.h */
struct ieee80211_hdr_3addr { 
  uint16_t  frame_control;  /* __le16 */
  uint16_t  duration_id;    /* __le16 */
  uint8_t   addr1[IEEE80211_MAC_ADDR_LEN];
  uint8_t   addr2[IEEE80211_MAC_ADDR_LEN];
  uint8_t   addr3[IEEE80211_MAC_ADDR_LEN];
  uint16_t  seq_ctrl;       /*__le16 */
} __attribute__ ((packed));
typedef struct ieee80211_hdr_3addr ieee80211_hdr;

typedef struct {
    ieee80211_radiotap_hdr  *radiotap_hdr;   /* frame metadata           */
    ieee80211_hdr           *mac_hdr;        /* link-layer header        */
    uint8_t                 *payload;        /* packet data              */
    //uint32_t              *checksum;       // Do we calculate this? or does the driver append this to our packet?

    uint8_t                 *__frame;        /* The actual data frame    */
} dxwifi_frame;

typedef struct {
    pcap_t* handle;         /* Session handle for PCAP  */
} dxwifi_transmitter;

/**
 *  Functions
 */

void init_dxwifi_frame(dxwifi_frame* frame, size_t block_size);
void teardown_dxwifi_frame(dxwifi_frame* frame);
void construct_dxwifi_header(dxwifi_frame* frame);
void construct_ieee80211_header(ieee80211_hdr* mac_hdr);
void construct_radiotap_header(ieee80211_radiotap_hdr* radiotap_hdr);

void init_transmitter(dxwifi_transmitter* transmitter, const char* dev_name);
void close_transmitter(dxwifi_transmitter* transmitter);

int transmit_file(dxwifi_transmitter* transmitter, int fd, size_t blocksize);

#endif // DXWIFI_H
