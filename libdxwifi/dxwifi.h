/**
 * DxWifi project includes, definitions, and types
 */


#ifndef LIBDXWIFI_H
#define LIBDXWIFI_H

#include <pcap.h>

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

// TODO this was defined arbitrarily. Needs review
#define DXWIFI_BLOCK_SIZE_MIN 0
#define DXWIFI_BLOCK_SIZE_MAX 1500

#define DXWIFI_PACKET_BUFFER_TIMEOUT 20

#define DXWIFI_TX_DURATION_ID 0xffff

#define DXWIFI_TX_HEADER_SIZE (sizeof(dxwifi_tx_radiotap_hdr) + sizeof(ieee80211_hdr))

#define DXWIFI_TX_RADIOTAP_PRESENCE_BIT_FIELD ( 0x1 << IEEE80211_RADIOTAP_FLAGS    \
                                              | 0x1 << IEEE80211_RADIOTAP_RATE     \
                                              | 0x1 << IEEE80211_RADIOTAP_TX_FLAGS)\


/************************
 *  Data structures
 ***********************/

/**
 *  The radiotap header is used to communicate to the driver information about 
 *  our packet. The header data itself is discarded before transmission.
 * 
 *  There are a lot of defined radiotap fields but most only make sense for 
 *  Rx packets. These are included becuase they help control injection and for 
 *  DxWifi on the OreSat most of these are constant values as well
 * 
 *  NOTE: Before modifying this struct be sure to read https://www.radiotap.org/ 
 *  for details. Fields are strictly ordered and aligned to natural boundries
 * 
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


/**
 * The transmitter is responsible for handling file transmission. It is the 
 * user's responsibility to fill the fields with the correct data they want 
 * for their transmission
 */
typedef struct {
    const char* device;                         /* 802.11 interface name    */
    size_t      block_size;                     /* Size in bytes to read    */

    uint8_t     rtap_flags;                     /* Radiotap flags           */
    uint8_t     rtap_rate;                      /* Radiotap data rate       */
    uint16_t    rtap_tx_flags;                  /* Radiotap Tx flags        */

    ieee80211_frame_control fctl;               /* Frame control settings   */
  
   /*
   *  The following addresses can have different intreptations depenging on the 
   *  state of the to_ds/from_ds flags in the frame control. By default we set
   *  to_ds to 0 and from_ds to 1.
   *  +-------+---------+-------------+-------------+-------------+-----------+
   *  | To DS | From DS | Address 1   | Address 2   | Address 3   | Address 4 |
   *  +-------+---------+-------------+-------------+-------------+-----------+
   *  |     0 |       0 | Destination | Source      | BSSID       | N/A       |
   *  |     0 |       1 | Destination | BSSID       | Source      | N/A       |
   *  |     1 |       0 | BSSID       | Source      | Destination | N/A       |
   *  |     1 |       1 | Receiver    | Transmitter | Destination | Source    |
   *  +-------+---------+-------------+-------------+-------------+-----------+
   */
    uint8_t     addr1[IEEE80211_MAC_ADDR_LEN];  /* Destination              */
    uint8_t     addr2[IEEE80211_MAC_ADDR_LEN];  /* BSSID                    */
    uint8_t     addr3[IEEE80211_MAC_ADDR_LEN];  /* Source                   */

    pcap_t*     __handle;                       /* Session handle for PCAP  */
} dxwifi_transmitter;

/**
 * The receiver is responsible for packet capture and unpacking the original
 * data.
 */
typedef struct {
    const char* device;                         /* 802.11 interface name    */

    // https://www.tcpdump.org/manpages/pcap.3pcap.html
    const char* filter;                         /* BPF Program string       */
    bool        optimize;                       /* Optimize compiled Filter?*/
    int         snaplen;                        /* Snapshot length in bytes */
    int         packet_buffer_timeout;

    pcap_t*     __handle;                       /* Session handle for PCAP  */
} dxwifi_receiver;

/************************
 *  Functions
 ***********************/

void init_transmitter(dxwifi_transmitter* transmitter);

int transmit_file(dxwifi_transmitter* transmitter, int fd);

void close_transmitter(dxwifi_transmitter* transmitter);


void init_receiver(dxwifi_receiver* receiver);

int receiver_capture(dxwifi_receiver* receiver);

void close_receiver(dxwifi_receiver* receiver);

#endif // LIBDXWIFI_H
