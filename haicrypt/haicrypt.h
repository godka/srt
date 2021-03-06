/*
 * SRT - Secure, Reliable, Transport
 * Copyright (c) 2017 Haivision Systems Inc.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; If not, see <http://www.gnu.org/licenses/>
 */

/*****************************************************************************
written by
   Haivision Systems Inc.

   2011-06-23 (jdube)
        HaiCrypt initial implementation.
   2014-03-11 (jdube)
        Adaptation for SRT.
*****************************************************************************/

#ifndef HAICRYPT_H
#define HAICRYPT_H

#include <sys/types.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// setup exports
#if defined WIN32 && !defined __MINGW__
#ifdef HAICRYPT_DYNAMIC
#ifdef HAICRYPT_EXPORTS
#define HAICRYPT_API __declspec(dllexport)
#else
#define HAICRYPT_API __declspec(dllimport)
#endif
#else
#define HAICRYPT_API
#endif
#else
#define HAICRYPT_API
#endif
/* 
 * Define (in Makefile) the HaiCrypt ciphers compiled in
 */
//#define HAICRYPT_USE_OPENSSL_EVP 1    /* Preferred for most cases */
//#define HAICRYPT_USE_OPENSSL_AES 1    /* Mandatory for key wrapping and prng */

typedef void *HaiCrypt_Cipher;

HAICRYPT_API HaiCrypt_Cipher HaiCryptCipher_Get_Instance (void);     /* Return a efault cipher instance */

HAICRYPT_API HaiCrypt_Cipher HaiCryptCipher_OpenSSL_EVP(void);       /* OpenSSL EVP interface (default to EVP_CTR) */
HAICRYPT_API HaiCrypt_Cipher HaiCryptCipher_OpenSSL_EVP_CBC(void);   /* OpenSSL EVP interface for AES-CBC */
HAICRYPT_API HaiCrypt_Cipher HaiCryptCipher_OpenSSL_EVP_CTR(void);   /* OpenSSL EVP interface for AES-CTR */
HAICRYPT_API HaiCrypt_Cipher HaiCryptCipher_OpenSSL_AES(void);   /* OpenSSL AES direct interface */


#define HAICRYPT_CIPHER_BLK_SZ      16  /* AES Block Size */

#define HAICRYPT_PWD_MAX_SZ         80  /* MAX password (for Password-based Key Derivation) */
#define HAICRYPT_KEY_MAX_SZ         32  /* MAX key */
#define HAICRYPT_SECRET_MAX_SZ      (HAICRYPT_PWD_MAX_SZ > HAICRYPT_KEY_MAX_SZ ? HAICRYPT_PWD_MAX_SZ : HAICRYPT_KEY_MAX_SZ)


#define HAICRYPT_SALT_SZ            16

#define HAICRYPT_WRAPKEY_SIGN_SZ    8       /* RFC3394 AES KeyWrap signature size */

#define HAICRYPT_PBKDF2_SALT_LEN    8       /* PKCS#5 PBKDF2 Password based key derivation salt length */
#define HAICRYPT_PBKDF2_ITER_CNT    2048    /* PKCS#5 PBKDF2 Password based key derivation iteration count */

#define HAICRYPT_TS_PKT_SZ          188     /* Transport Stream packet size */

typedef struct {
#define HAICRYPT_SECTYP_UNDEF       0
#define HAICRYPT_SECTYP_PRESHARED   1       /* Preshared KEK */
#define HAICRYPT_SECTYP_PASSPHRASE  2       /* Password */
        unsigned        typ;
        size_t          len;
        unsigned char   str[HAICRYPT_SECRET_MAX_SZ];
}HaiCrypt_Secret;

typedef struct {
#define HAICRYPT_CFG_F_TX       0x01        /* !TX -> RX */
#define HAICRYPT_CFG_F_CRYPTO   0x02        /* Perform crypto Tx:Encrypt Rx:Decrypt */
#define HAICRYPT_CFG_F_FEC      0x04        /* Do Forward Error Correction */
        unsigned        flags;

        HaiCrypt_Secret secret;             /* Security Association */

        HaiCrypt_Cipher cipher;             /* Media Stream cipher implementation */
#define HAICRYPT_DEF_KEY_LENGTH 16          /* default key length (bytes) */
        size_t  key_len;                    /* SEK length (bytes) */
#define HAICRYPT_DEF_DATA_MAX_LENGTH 1500   /* default packet data length (bytes) */
        size_t  data_max_len;               /* Maximum data_len passed to HaiCrypt (bytes) */

#define HAICRYPT_XPT_STANDALONE 0
#define HAICRYPT_XPT_SRT        1
        int             xport;

#define HAICRYPT_DEF_KM_TX_PERIOD 1000          /* Keying Material Default Tx Period (msec) */
        unsigned int    km_tx_period_ms;        /* Keying Material Tx period (msec) */
#define HAICRYPT_DEF_KM_REFRESH_RATE 0x1000000  /* Keying Material Default Refresh Rate (pkts) */
        unsigned int    km_refresh_rate_pkt;    /* Keying Material Refresh Rate (pkts) */
#define HAICRYPT_DEF_KM_PRE_ANNOUNCE 0x1000     /* Keying Material Default Pre/Post Announce (pkts) */
        unsigned int    km_pre_announce_pkt;    /* Keying Material Pre/Post Announce (pkts) */
}HaiCrypt_Cfg;

typedef void *HaiCrypt_Handle;

HAICRYPT_API int  HaiCrypt_SetLogLevel(int level, int logfa);

HAICRYPT_API int  HaiCrypt_Create(HaiCrypt_Cfg *cfg, HaiCrypt_Handle *phhc);
HAICRYPT_API int  HaiCrypt_Close(HaiCrypt_Handle hhc);
HAICRYPT_API int  HaiCrypt_Tx_GetBuf(HaiCrypt_Handle hhc, size_t data_len, unsigned char **in_p);
HAICRYPT_API int  HaiCrypt_Tx_Process(HaiCrypt_Handle hhc, unsigned char *in, size_t in_len,
        void *out_p[], size_t out_len_p[], int maxout);
HAICRYPT_API int  HaiCrypt_Rx_Process(HaiCrypt_Handle hhc, unsigned char *in, size_t in_len,
        void *out_p[], size_t out_len_p[], int maxout);

HAICRYPT_API int  HaiCrypt_Tx_GetKeyFlags(HaiCrypt_Handle hhc);
HAICRYPT_API int  HaiCrypt_Tx_ManageKeys(HaiCrypt_Handle hhc, void *out_p[], size_t out_len_p[], int maxout);
HAICRYPT_API int  HaiCrypt_Tx_Data(HaiCrypt_Handle hhc, unsigned char *pfx, unsigned char *data, size_t data_len);
HAICRYPT_API int  HaiCrypt_Rx_Data(HaiCrypt_Handle hhc, unsigned char *pfx, unsigned char *data, size_t data_len);

#ifdef __cplusplus
}
#endif

#endif /* HAICRYPT_H */
