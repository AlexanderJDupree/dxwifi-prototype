/**
 * DxWifi Transmitter is responsible to reading blocks of arbitrary data and
 * constructing an the required headers for injection
 */

#ifndef LIBDXWIFI_TRANSMITTER_H
#define LIBDXWIFI_TRANSMITTER_H

#include <pcap.h>

#include <libdxwifi/details/ieee80211.h>

/************************
 *  Constants
 ***********************/

#define DXWIFI_TX_DURATION_ID 0xffff

#define DXWIFI_TX_HEADER_SIZE (sizeof(dxwifi_tx_radiotap_hdr) + sizeof(ieee80211_hdr))

#define DXWIFI_TX_PREAMBLE_SIZE 256
#define DXWIFI_TX_PREAMBLE_VALUE 0xff

#define DXWIFI_TX_EOT_SIZE 256
#define DXWIFI_TX_EOT_VALUE 0xaa

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
    struct ieee80211_radiotap_header    hdr;      /* packed radiotap header   */
    uint8_t                             flags;    /* frame flags              */
    uint8_t                             rate;     /* data rate (500Kbps)      */
    uint16_t                            tx_flags; /* transmission flags       */
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
    dxwifi_tx_radiotap_hdr  *radiotap_hdr;  /* frame metadata               */
    ieee80211_hdr           *mac_hdr;       /* link-layer header            */
    uint8_t                 *payload;       /* packet data                  */

    uint8_t                 *__frame;       /* The actual data frame        */
} dxwifi_tx_frame;


/**
 * The transmitter is responsible for handling file transmission. It is the 
 * user's responsibility to fill the fields with the correct data they want 
 * for their transmission
 */
typedef struct {
    const char* device;                         /* 802.11 interface name    */
    size_t      block_size;                     /* Size in bytes to read    */
    int         transmit_timeout;               /* Seconds to wait for read */


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
    uint8_t         addr1[IEEE80211_MAC_ADDR_LEN];
    uint8_t         addr2[IEEE80211_MAC_ADDR_LEN];
    uint8_t         addr3[IEEE80211_MAC_ADDR_LEN];

    volatile bool   __activated;                /* Currently transmitting?  */
    pcap_t*         __handle;                   /* Session handle for PCAP  */
} dxwifi_transmitter;


/************************
 *  Functions
 ***********************/

void init_transmitter(dxwifi_transmitter* transmitter);

int start_transmission(dxwifi_transmitter* transmitter, int fd);

void stop_transmission(dxwifi_transmitter* transmitter);

void close_transmitter(dxwifi_transmitter* transmitter);


#endif // LIBDXWIFI_TRANSMITTER_H
