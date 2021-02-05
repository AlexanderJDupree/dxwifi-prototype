/*
 * Copyright (c) 2017		Intel Deutschland GmbH
 * Copyright (c) 2018-2019	Intel Corporation
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 */

/**
 * The following code has been ported from https://github.com/torvalds/linux
 * 
 * This file defines a ad-hoc subset of the 802.11 standard. Only components 
 * that directly effect the dxwifi project are defined and may not be 
 * completely faithful to the standard.
 * 
 */

#ifndef DXWIFI_IEEE80211_H
#define DXWIFI_IEEE80211_H

#include <endian.h>
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
 * struct ieee82011_radiotap_header - base radiotap header
 */
struct ieee80211_radiotap_header {
    /**
     * @it_version: radiotap version, always 0
     */
    uint8_t it_version;

    /**
     * @it_pad: padding (or alignment)
     */
    uint8_t it_pad;

    /**
     * @it_len: overall radiotap header length
     */
    uint16_t it_len;

    /**
     * @it_present: (first) present word
     */
    uint32_t it_present;
} __attribute__((__packed__));

/* version is always 0 */
#define PKTHDR_RADIOTAP_VERSION	0

/* see the radiotap website for the descriptions */
enum ieee80211_radiotap_presence {
    IEEE80211_RADIOTAP_TSFT = 0,
    IEEE80211_RADIOTAP_FLAGS = 1,
    IEEE80211_RADIOTAP_RATE = 2,
    IEEE80211_RADIOTAP_CHANNEL = 3,
    IEEE80211_RADIOTAP_FHSS = 4,
    IEEE80211_RADIOTAP_DBM_ANTSIGNAL = 5,
    IEEE80211_RADIOTAP_DBM_ANTNOISE = 6,
    IEEE80211_RADIOTAP_LOCK_QUALITY = 7,
    IEEE80211_RADIOTAP_TX_ATTENUATION = 8,
    IEEE80211_RADIOTAP_DB_TX_ATTENUATION = 9,
    IEEE80211_RADIOTAP_DBM_TX_POWER = 10,
    IEEE80211_RADIOTAP_ANTENNA = 11,
    IEEE80211_RADIOTAP_DB_ANTSIGNAL = 12,
    IEEE80211_RADIOTAP_DB_ANTNOISE = 13,
    IEEE80211_RADIOTAP_RX_FLAGS = 14,
    IEEE80211_RADIOTAP_TX_FLAGS = 15,
    IEEE80211_RADIOTAP_RTS_RETRIES = 16,
    IEEE80211_RADIOTAP_DATA_RETRIES = 17,
    /* 18 is XChannel, but it's not defined yet */
    IEEE80211_RADIOTAP_MCS = 19,
    IEEE80211_RADIOTAP_AMPDU_STATUS = 20,
    IEEE80211_RADIOTAP_VHT = 21,
    IEEE80211_RADIOTAP_TIMESTAMP = 22,
    IEEE80211_RADIOTAP_HE = 23,
    IEEE80211_RADIOTAP_HE_MU = 24,
    IEEE80211_RADIOTAP_ZERO_LEN_PSDU = 26,
    IEEE80211_RADIOTAP_LSIG = 27,

    /* valid in every it_present bitmap, even vendor namespaces */
    IEEE80211_RADIOTAP_RADIOTAP_NAMESPACE = 29,
    IEEE80211_RADIOTAP_VENDOR_NAMESPACE = 30,
    IEEE80211_RADIOTAP_EXT = 31
};

/* for IEEE80211_RADIOTAP_FLAGS */
enum ieee80211_radiotap_flags {
    IEEE80211_RADIOTAP_F_CFP = 0x01,
    IEEE80211_RADIOTAP_F_SHORTPRE = 0x02,
    IEEE80211_RADIOTAP_F_WEP = 0x04,
    IEEE80211_RADIOTAP_F_FRAG = 0x08,
    IEEE80211_RADIOTAP_F_FCS = 0x10,
    IEEE80211_RADIOTAP_F_DATAPAD = 0x20,
    IEEE80211_RADIOTAP_F_BADFCS = 0x40,
};

/* for IEEE80211_RADIOTAP_CHANNEL */
enum ieee80211_radiotap_channel_flags {
    IEEE80211_CHAN_CCK = 0x0020,
    IEEE80211_CHAN_OFDM = 0x0040,
    IEEE80211_CHAN_2GHZ = 0x0080,
    IEEE80211_CHAN_5GHZ = 0x0100,
    IEEE80211_CHAN_DYN = 0x0400,
    IEEE80211_CHAN_HALF = 0x4000,
    IEEE80211_CHAN_QUARTER = 0x8000,
};

/* for IEEE80211_RADIOTAP_RX_FLAGS */
enum ieee80211_radiotap_rx_flags {
    IEEE80211_RADIOTAP_F_RX_BADPLCP = 0x0002,
};

/* for IEEE80211_RADIOTAP_TX_FLAGS */
enum ieee80211_radiotap_tx_flags {
    IEEE80211_RADIOTAP_F_TX_FAIL = 0x0001,
    IEEE80211_RADIOTAP_F_TX_CTS = 0x0002,
    IEEE80211_RADIOTAP_F_TX_RTS = 0x0004,
    IEEE80211_RADIOTAP_F_TX_NOACK = 0x0008,
    IEEE80211_RADIOTAP_F_TX_NOSEQNO = 0x0010,
    IEEE80211_RADIOTAP_F_TX_ORDER = 0x0020,
};

/* for IEEE80211_RADIOTAP_MCS "have" flags */
enum ieee80211_radiotap_mcs_have {
    IEEE80211_RADIOTAP_MCS_HAVE_BW = 0x01,
    IEEE80211_RADIOTAP_MCS_HAVE_MCS = 0x02,
    IEEE80211_RADIOTAP_MCS_HAVE_GI = 0x04,
    IEEE80211_RADIOTAP_MCS_HAVE_FMT = 0x08,
    IEEE80211_RADIOTAP_MCS_HAVE_FEC = 0x10,
    IEEE80211_RADIOTAP_MCS_HAVE_STBC = 0x20,
};

enum ieee80211_radiotap_mcs_flags {
    IEEE80211_RADIOTAP_MCS_BW_MASK = 0x03,
    IEEE80211_RADIOTAP_MCS_BW_20 = 0,
    IEEE80211_RADIOTAP_MCS_BW_40 = 1,
    IEEE80211_RADIOTAP_MCS_BW_20L = 2,
    IEEE80211_RADIOTAP_MCS_BW_20U = 3,

    IEEE80211_RADIOTAP_MCS_SGI = 0x04,
    IEEE80211_RADIOTAP_MCS_FMT_GF = 0x08,
    IEEE80211_RADIOTAP_MCS_FEC_LDPC = 0x10,
    IEEE80211_RADIOTAP_MCS_STBC_MASK = 0x60,
    IEEE80211_RADIOTAP_MCS_STBC_1 = 1,
    IEEE80211_RADIOTAP_MCS_STBC_2 = 2,
    IEEE80211_RADIOTAP_MCS_STBC_3 = 3,
    IEEE80211_RADIOTAP_MCS_STBC_SHIFT = 5,
};

/* for IEEE80211_RADIOTAP_AMPDU_STATUS */
enum ieee80211_radiotap_ampdu_flags {
    IEEE80211_RADIOTAP_AMPDU_REPORT_ZEROLEN = 0x0001,
    IEEE80211_RADIOTAP_AMPDU_IS_ZEROLEN = 0x0002,
    IEEE80211_RADIOTAP_AMPDU_LAST_KNOWN = 0x0004,
    IEEE80211_RADIOTAP_AMPDU_IS_LAST = 0x0008,
    IEEE80211_RADIOTAP_AMPDU_DELIM_CRC_ERR = 0x0010,
    IEEE80211_RADIOTAP_AMPDU_DELIM_CRC_KNOWN = 0x0020,
    IEEE80211_RADIOTAP_AMPDU_EOF = 0x0040,
    IEEE80211_RADIOTAP_AMPDU_EOF_KNOWN = 0x0080,
};

/* for IEEE80211_RADIOTAP_VHT */
enum ieee80211_radiotap_vht_known {
    IEEE80211_RADIOTAP_VHT_KNOWN_STBC = 0x0001,
    IEEE80211_RADIOTAP_VHT_KNOWN_TXOP_PS_NA = 0x0002,
    IEEE80211_RADIOTAP_VHT_KNOWN_GI = 0x0004,
    IEEE80211_RADIOTAP_VHT_KNOWN_SGI_NSYM_DIS = 0x0008,
    IEEE80211_RADIOTAP_VHT_KNOWN_LDPC_EXTRA_OFDM_SYM = 0x0010,
    IEEE80211_RADIOTAP_VHT_KNOWN_BEAMFORMED = 0x0020,
    IEEE80211_RADIOTAP_VHT_KNOWN_BANDWIDTH = 0x0040,
    IEEE80211_RADIOTAP_VHT_KNOWN_GROUP_ID = 0x0080,
    IEEE80211_RADIOTAP_VHT_KNOWN_PARTIAL_AID = 0x0100,
};

enum ieee80211_radiotap_vht_flags {
    IEEE80211_RADIOTAP_VHT_FLAG_STBC = 0x01,
    IEEE80211_RADIOTAP_VHT_FLAG_TXOP_PS_NA = 0x02,
    IEEE80211_RADIOTAP_VHT_FLAG_SGI = 0x04,
    IEEE80211_RADIOTAP_VHT_FLAG_SGI_NSYM_M10_9 = 0x08,
    IEEE80211_RADIOTAP_VHT_FLAG_LDPC_EXTRA_OFDM_SYM = 0x10,
    IEEE80211_RADIOTAP_VHT_FLAG_BEAMFORMED = 0x20,
};

enum ieee80211_radiotap_vht_coding {
    IEEE80211_RADIOTAP_CODING_LDPC_USER0 = 0x01,
    IEEE80211_RADIOTAP_CODING_LDPC_USER1 = 0x02,
    IEEE80211_RADIOTAP_CODING_LDPC_USER2 = 0x04,
    IEEE80211_RADIOTAP_CODING_LDPC_USER3 = 0x08,
};

/* for IEEE80211_RADIOTAP_TIMESTAMP */
enum ieee80211_radiotap_timestamp_unit_spos {
    IEEE80211_RADIOTAP_TIMESTAMP_UNIT_MASK = 0x000F,
    IEEE80211_RADIOTAP_TIMESTAMP_UNIT_MS = 0x0000,
    IEEE80211_RADIOTAP_TIMESTAMP_UNIT_US = 0x0001,
    IEEE80211_RADIOTAP_TIMESTAMP_UNIT_NS = 0x0003,
    IEEE80211_RADIOTAP_TIMESTAMP_SPOS_MASK = 0x00F0,
    IEEE80211_RADIOTAP_TIMESTAMP_SPOS_BEGIN_MDPU = 0x0000,
    IEEE80211_RADIOTAP_TIMESTAMP_SPOS_PLCP_SIG_ACQ = 0x0010,
    IEEE80211_RADIOTAP_TIMESTAMP_SPOS_EO_PPDU = 0x0020,
    IEEE80211_RADIOTAP_TIMESTAMP_SPOS_EO_MPDU = 0x0030,
    IEEE80211_RADIOTAP_TIMESTAMP_SPOS_UNKNOWN = 0x00F0,
};

enum ieee80211_radiotap_timestamp_flags {
    IEEE80211_RADIOTAP_TIMESTAMP_FLAG_64BIT = 0x00,
    IEEE80211_RADIOTAP_TIMESTAMP_FLAG_32BIT = 0x01,
    IEEE80211_RADIOTAP_TIMESTAMP_FLAG_ACCURACY = 0x02,
};

struct ieee80211_radiotap_he {
    uint16_t data1, data2, data3, data4, data5, data6;
};

enum ieee80211_radiotap_he_bits {
    IEEE80211_RADIOTAP_HE_DATA1_FORMAT_MASK		= 3,
    IEEE80211_RADIOTAP_HE_DATA1_FORMAT_SU		= 0,
    IEEE80211_RADIOTAP_HE_DATA1_FORMAT_EXT_SU	= 1,
    IEEE80211_RADIOTAP_HE_DATA1_FORMAT_MU		= 2,
    IEEE80211_RADIOTAP_HE_DATA1_FORMAT_TRIG		= 3,

    IEEE80211_RADIOTAP_HE_DATA1_BSS_COLOR_KNOWN	= 0x0004,
    IEEE80211_RADIOTAP_HE_DATA1_BEAM_CHANGE_KNOWN	= 0x0008,
    IEEE80211_RADIOTAP_HE_DATA1_UL_DL_KNOWN		= 0x0010,
    IEEE80211_RADIOTAP_HE_DATA1_DATA_MCS_KNOWN	= 0x0020,
    IEEE80211_RADIOTAP_HE_DATA1_DATA_DCM_KNOWN	= 0x0040,
    IEEE80211_RADIOTAP_HE_DATA1_CODING_KNOWN	= 0x0080,
    IEEE80211_RADIOTAP_HE_DATA1_LDPC_XSYMSEG_KNOWN	= 0x0100,
    IEEE80211_RADIOTAP_HE_DATA1_STBC_KNOWN		= 0x0200,
    IEEE80211_RADIOTAP_HE_DATA1_SPTL_REUSE_KNOWN	= 0x0400,
    IEEE80211_RADIOTAP_HE_DATA1_SPTL_REUSE2_KNOWN	= 0x0800,
    IEEE80211_RADIOTAP_HE_DATA1_SPTL_REUSE3_KNOWN	= 0x1000,
    IEEE80211_RADIOTAP_HE_DATA1_SPTL_REUSE4_KNOWN	= 0x2000,
    IEEE80211_RADIOTAP_HE_DATA1_BW_RU_ALLOC_KNOWN	= 0x4000,
    IEEE80211_RADIOTAP_HE_DATA1_DOPPLER_KNOWN	= 0x8000,

    IEEE80211_RADIOTAP_HE_DATA2_PRISEC_80_KNOWN	= 0x0001,
    IEEE80211_RADIOTAP_HE_DATA2_GI_KNOWN		= 0x0002,
    IEEE80211_RADIOTAP_HE_DATA2_NUM_LTF_SYMS_KNOWN	= 0x0004,
    IEEE80211_RADIOTAP_HE_DATA2_PRE_FEC_PAD_KNOWN	= 0x0008,
    IEEE80211_RADIOTAP_HE_DATA2_TXBF_KNOWN		= 0x0010,
    IEEE80211_RADIOTAP_HE_DATA2_PE_DISAMBIG_KNOWN	= 0x0020,
    IEEE80211_RADIOTAP_HE_DATA2_TXOP_KNOWN		= 0x0040,
    IEEE80211_RADIOTAP_HE_DATA2_MIDAMBLE_KNOWN	= 0x0080,
    IEEE80211_RADIOTAP_HE_DATA2_RU_OFFSET		= 0x3f00,
    IEEE80211_RADIOTAP_HE_DATA2_RU_OFFSET_KNOWN	= 0x4000,
    IEEE80211_RADIOTAP_HE_DATA2_PRISEC_80_SEC	= 0x8000,

    IEEE80211_RADIOTAP_HE_DATA3_BSS_COLOR		= 0x003f,
    IEEE80211_RADIOTAP_HE_DATA3_BEAM_CHANGE		= 0x0040,
    IEEE80211_RADIOTAP_HE_DATA3_UL_DL		= 0x0080,
    IEEE80211_RADIOTAP_HE_DATA3_DATA_MCS		= 0x0f00,
    IEEE80211_RADIOTAP_HE_DATA3_DATA_DCM		= 0x1000,
    IEEE80211_RADIOTAP_HE_DATA3_CODING		= 0x2000,
    IEEE80211_RADIOTAP_HE_DATA3_LDPC_XSYMSEG	= 0x4000,
    IEEE80211_RADIOTAP_HE_DATA3_STBC		= 0x8000,

    IEEE80211_RADIOTAP_HE_DATA4_SU_MU_SPTL_REUSE	= 0x000f,
    IEEE80211_RADIOTAP_HE_DATA4_MU_STA_ID		= 0x7ff0,
    IEEE80211_RADIOTAP_HE_DATA4_TB_SPTL_REUSE1	= 0x000f,
    IEEE80211_RADIOTAP_HE_DATA4_TB_SPTL_REUSE2	= 0x00f0,
    IEEE80211_RADIOTAP_HE_DATA4_TB_SPTL_REUSE3	= 0x0f00,
    IEEE80211_RADIOTAP_HE_DATA4_TB_SPTL_REUSE4	= 0xf000,

    IEEE80211_RADIOTAP_HE_DATA5_DATA_BW_RU_ALLOC	= 0x000f,
        IEEE80211_RADIOTAP_HE_DATA5_DATA_BW_RU_ALLOC_20MHZ	= 0,
        IEEE80211_RADIOTAP_HE_DATA5_DATA_BW_RU_ALLOC_40MHZ	= 1,
        IEEE80211_RADIOTAP_HE_DATA5_DATA_BW_RU_ALLOC_80MHZ	= 2,
        IEEE80211_RADIOTAP_HE_DATA5_DATA_BW_RU_ALLOC_160MHZ	= 3,
        IEEE80211_RADIOTAP_HE_DATA5_DATA_BW_RU_ALLOC_26T	= 4,
        IEEE80211_RADIOTAP_HE_DATA5_DATA_BW_RU_ALLOC_52T	= 5,
        IEEE80211_RADIOTAP_HE_DATA5_DATA_BW_RU_ALLOC_106T	= 6,
        IEEE80211_RADIOTAP_HE_DATA5_DATA_BW_RU_ALLOC_242T	= 7,
        IEEE80211_RADIOTAP_HE_DATA5_DATA_BW_RU_ALLOC_484T	= 8,
        IEEE80211_RADIOTAP_HE_DATA5_DATA_BW_RU_ALLOC_996T	= 9,
        IEEE80211_RADIOTAP_HE_DATA5_DATA_BW_RU_ALLOC_2x996T	= 10,

    IEEE80211_RADIOTAP_HE_DATA5_GI			= 0x0030,
        IEEE80211_RADIOTAP_HE_DATA5_GI_0_8			= 0,
        IEEE80211_RADIOTAP_HE_DATA5_GI_1_6			= 1,
        IEEE80211_RADIOTAP_HE_DATA5_GI_3_2			= 2,

    IEEE80211_RADIOTAP_HE_DATA5_LTF_SIZE		= 0x00c0,
        IEEE80211_RADIOTAP_HE_DATA5_LTF_SIZE_UNKNOWN		= 0,
        IEEE80211_RADIOTAP_HE_DATA5_LTF_SIZE_1X			= 1,
        IEEE80211_RADIOTAP_HE_DATA5_LTF_SIZE_2X			= 2,
        IEEE80211_RADIOTAP_HE_DATA5_LTF_SIZE_4X			= 3,
    IEEE80211_RADIOTAP_HE_DATA5_NUM_LTF_SYMS	= 0x0700,
    IEEE80211_RADIOTAP_HE_DATA5_PRE_FEC_PAD		= 0x3000,
    IEEE80211_RADIOTAP_HE_DATA5_TXBF		= 0x4000,
    IEEE80211_RADIOTAP_HE_DATA5_PE_DISAMBIG		= 0x8000,

    IEEE80211_RADIOTAP_HE_DATA6_NSTS		= 0x000f,
    IEEE80211_RADIOTAP_HE_DATA6_DOPPLER		= 0x0010,
    IEEE80211_RADIOTAP_HE_DATA6_TB_PPDU_BW_KNOWN	= 0x0020,
    IEEE80211_RADIOTAP_HE_DATA6_TB_PPDU_BW		= 0x00c0,
        IEEE80211_RADIOTAP_HE_DATA6_TB_PPDU_BW_20MHZ	= 0,
        IEEE80211_RADIOTAP_HE_DATA6_TB_PPDU_BW_40MHZ	= 1,
        IEEE80211_RADIOTAP_HE_DATA6_TB_PPDU_BW_80MHZ	= 2,
        IEEE80211_RADIOTAP_HE_DATA6_TB_PPDU_BW_160MHZ	= 3,
    IEEE80211_RADIOTAP_HE_DATA6_TXOP		= 0x7f00,
    IEEE80211_RADIOTAP_HE_DATA6_MIDAMBLE_PDCTY	= 0x8000,
};

struct ieee80211_radiotap_he_mu {
    uint16_t flags1, flags2;
    uint8_t ru_ch1[4];
    uint8_t ru_ch2[4];
};

enum ieee80211_radiotap_he_mu_bits {
    IEEE80211_RADIOTAP_HE_MU_FLAGS1_SIG_B_MCS		= 0x000f,
    IEEE80211_RADIOTAP_HE_MU_FLAGS1_SIG_B_MCS_KNOWN		= 0x0010,
    IEEE80211_RADIOTAP_HE_MU_FLAGS1_SIG_B_DCM		= 0x0020,
    IEEE80211_RADIOTAP_HE_MU_FLAGS1_SIG_B_DCM_KNOWN		= 0x0040,
    IEEE80211_RADIOTAP_HE_MU_FLAGS1_CH2_CTR_26T_RU_KNOWN	= 0x0080,
    IEEE80211_RADIOTAP_HE_MU_FLAGS1_CH1_RU_KNOWN		= 0x0100,
    IEEE80211_RADIOTAP_HE_MU_FLAGS1_CH2_RU_KNOWN		= 0x0200,
    IEEE80211_RADIOTAP_HE_MU_FLAGS1_CH1_CTR_26T_RU_KNOWN	= 0x1000,
    IEEE80211_RADIOTAP_HE_MU_FLAGS1_CH1_CTR_26T_RU		= 0x2000,
    IEEE80211_RADIOTAP_HE_MU_FLAGS1_SIG_B_COMP_KNOWN	= 0x4000,
    IEEE80211_RADIOTAP_HE_MU_FLAGS1_SIG_B_SYMS_USERS_KNOWN	= 0x8000,

    IEEE80211_RADIOTAP_HE_MU_FLAGS2_BW_FROM_SIG_A_BW	= 0x0003,
        IEEE80211_RADIOTAP_HE_MU_FLAGS2_BW_FROM_SIG_A_BW_20MHZ	= 0x0000,
        IEEE80211_RADIOTAP_HE_MU_FLAGS2_BW_FROM_SIG_A_BW_40MHZ	= 0x0001,
        IEEE80211_RADIOTAP_HE_MU_FLAGS2_BW_FROM_SIG_A_BW_80MHZ	= 0x0002,
        IEEE80211_RADIOTAP_HE_MU_FLAGS2_BW_FROM_SIG_A_BW_160MHZ	= 0x0003,
    IEEE80211_RADIOTAP_HE_MU_FLAGS2_BW_FROM_SIG_A_BW_KNOWN	= 0x0004,
    IEEE80211_RADIOTAP_HE_MU_FLAGS2_SIG_B_COMP		= 0x0008,
    IEEE80211_RADIOTAP_HE_MU_FLAGS2_SIG_B_SYMS_USERS	= 0x00f0,
    IEEE80211_RADIOTAP_HE_MU_FLAGS2_PUNC_FROM_SIG_A_BW	= 0x0300,
    IEEE80211_RADIOTAP_HE_MU_FLAGS2_PUNC_FROM_SIG_A_BW_KNOWN= 0x0400,
    IEEE80211_RADIOTAP_HE_MU_FLAGS2_CH2_CTR_26T_RU		= 0x0800,
};

enum ieee80211_radiotap_lsig_data1 {
    IEEE80211_RADIOTAP_LSIG_DATA1_RATE_KNOWN		= 0x0001,
    IEEE80211_RADIOTAP_LSIG_DATA1_LENGTH_KNOWN		= 0x0002,
};

enum ieee80211_radiotap_lsig_data2 {
    IEEE80211_RADIOTAP_LSIG_DATA2_RATE			= 0x000f,
    IEEE80211_RADIOTAP_LSIG_DATA2_LENGTH			= 0xfff0,
};

struct ieee80211_radiotap_lsig {
    uint16_t data1, data2;
};

enum ieee80211_radiotap_zero_len_psdu_type {
    IEEE80211_RADIOTAP_ZERO_LEN_PSDU_SOUNDING		= 0,
    IEEE80211_RADIOTAP_ZERO_LEN_PSDU_NOT_CAPTURED		= 1,
    IEEE80211_RADIOTAP_ZERO_LEN_PSDU_VENDOR			= 0xff,
};

#endif // DXWIFI_IEEE80211_H
