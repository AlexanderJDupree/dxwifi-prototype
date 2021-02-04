/**
 * DxWifi project includes, definitions, and types
 */


#ifndef DXWIFI_H
#define DXWIFI_H

#include <pcap.h>
#include <radiotap/radiotap.h>

#include <dxwifi/ieee80211.h>

/************************
 *  Versioning
 ***********************/

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


/************************
 *  Default Arguments
 ***********************/

#define DXWIFI_DFLT_DEVICE 			            "mon0"
#define DXWIFI_DFLT_BLK_SIZE 			          256
#define DXWIFI_DFLT_PACKET_BUFFER_TIMEOUT 	20

/************************
 *  Limits
 ***********************/

// https://www.tcpdump.org/manpages/pcap.3pcap.html
#define SNAPLEN_MAX 65535

// TODO this were defined arbitrarily. Needs review
#define DXWIFI_BLOCK_SIZE_MIN 0
#define DXWIFI_BLOCK_SIZE_MAX 1500

#define DXWIFI_TX_HEADER_SIZE (sizeof(dxwifi_tx_radiotap_hdr) + sizeof(ieee80211_hdr))


/************************
 *  Data structures
 ***********************/

/**
 * The radiotap header is used to communicate to the driver information about 
 * out packet. The header data itself is discarded before transmission.
 */
typedef struct  __attribute__((packed)) {
  struct ieee80211_radiotap_header  hdr;      /* packed radiotap header */
  uint8_t                           flags;    /* frame flags            */
  uint8_t                           rate;     /* data rate (500Kbps)    */
  uint16_t                          tx_flags; /* transmission flags     */
} dxwifi_tx_radiotap_hdr;


/**
 *  The DxWifi frame structure looks like this:
 * 
 * 
 *    [ radiotap header + radiotap fields ] <--
 *    [         ieee80211 header          ]   |- All allocated on the same block
 *    [             payload               ]   |
 *    [       frame check sequence        ] <--
 *   
 * 
 *  The fields in the dxwifi_tx_frame struct simply point to the correct area in
 *  __frame array. We fill in the correct data for each header and then 
 *  transmit the entire frame of data. Note, there isn't a struct field for the
 *  frame check sequence (FCS) since the driver will populate that field for us.
 *  Thus, you should not manipulate the __frame field unless you know what 
 *  you're doing. 
 * 
 */
typedef struct { 
    dxwifi_tx_radiotap_hdr  *radiotap_hdr;  /* frame metadata           */
    ieee80211_hdr           *mac_hdr;       /* link-layer header        */
    uint8_t                 *payload;       /* packet data              */

    uint8_t                 *__frame;       /* The actual data frame    */
} dxwifi_tx_frame;


typedef struct {
    pcap_t* handle;                         /* Session handle for PCAP  */
} dxwifi_transmitter;


/************************
 *  Functions
 ***********************/

void init_dxwifi_tx_frame(dxwifi_tx_frame* frame, size_t block_size);

void teardown_dxwifi_tx_frame(dxwifi_tx_frame* frame);

void construct_dxwifi_header(dxwifi_tx_frame* frame);

void construct_ieee80211_header(ieee80211_hdr* mac_hdr);

void construct_radiotap_header(dxwifi_tx_radiotap_hdr* radiotap_hdr);

void init_transmitter(dxwifi_transmitter* transmitter, const char* dev_name);

void close_transmitter(dxwifi_transmitter* transmitter);

int transmit_file(dxwifi_transmitter* transmitter, int fd, size_t blocksize);


#endif // DXWIFI_H
