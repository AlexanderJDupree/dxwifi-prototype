/**
 * This file defines a ad-hoc subset of the 802.11 standard. Only components 
 * that directly effect the dxwifi project are defined and may not be 
 * completely faithful to the standard.
 */

#ifndef DXWIFI_IEEE80211_H
#define DXWIFI_IEEE80211_H

#include <stdint.h>

// https://networkengineering.stackexchange.com/questions/32970/what-is-the-802-11-mtu
#define IEEE80211_MTU_MAX_LEN 2304

#define IEEE80211_MAC_ADDR_LEN 6

#define IEEE80211_FCS_SIZE 4

#define IEEE80211_RADIOTAP_MAJOR_VERSION 0

// Defined in github.com/torvalds/linux/include/linux/ieee80211.h 
struct ieee80211_hdr_3addr { 
    uint16_t    frame_control;  /* __le16 */
    uint16_t    duration_id;    /* __le16 */
    uint8_t     addr1[IEEE80211_MAC_ADDR_LEN];
    uint8_t     addr2[IEEE80211_MAC_ADDR_LEN];
    uint8_t     addr3[IEEE80211_MAC_ADDR_LEN];
    uint16_t    seq_ctrl;       /*__le16 */
} __attribute__ ((packed));
typedef struct ieee80211_hdr_3addr ieee80211_hdr;

/**
 * ieee80211_get_radiotap_len - get radiotap header length
static inline u16 ieee80211_get_radiotap_len(const char *data)
{
	struct ieee80211_radiotap_header *hdr = (void *)data;

	return get_unaligned_le16(&hdr->it_len);
}
 */

#endif // DXWIFI_IEEE80211_H
