/**
 * @file ex_light.c
 * @author NXP Semiconductors
 * @version 1.0
 * @par License
 * Copyright 2016 NXP
 *
 * This software is owned or controlled by NXP and may only be used
 * strictly in accordance with the applicable license terms.  By expressly
 * accepting such terms or by downloading, installing, activating and/or
 * otherwise using the software, you are agreeing that you have read, and
 * that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you
 * may not retain, install, activate or otherwise use the software.
 *
 * @par Description
 * Lightweight Example invocation not relying on any crypto capabilities of the Host Platform
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// #include "a71ch_ex.h"
#include "sm_printf.h"
#include "a71_debug.h"
#include "sm_types.h"
#include "sm_apdu.h"
#include "ax_util.h"
#include "tst_sm_util.h"
#include "tst_a71ch_util.h"
// #include "tstHostCrypto.h"

static U8 exAesRfc3394Precooked(U8 initMode);

/**
 * Demonstrate wrapped setting/retrieving of symmetric keys:
 * - ::exAesRfc3394Precooked
 */
U8 exLight()
{
    U8 result = 1;
    PRINTF( "\r\n-----------\r\nStart exLight()\r\n------------\r\n");

    // DEV_ClearChannelState();

    // ** Without channel encryption **
    // RFC3394 wrapping (using prepared values)
    result &= exAesRfc3394Precooked(INIT_MODE_RESET);

    // overall result
    PRINTF( "\r\n-----------\r\nEnd exLight(), result = %s\r\n------------\r\n", ((result == 1)? "OK": "FAILED"));

    return result;
}

/**
 * Demonstrate setting wrapped AES keys.
 * No Host Side crypto is required because test vectors have been pre-calculated ('precooked').
 *
 * @param[in] initMode Visit the documentation of ::a71chInitModule for more information on this parameter
 */
static U8 exAesRfc3394Precooked(U8 initMode)
{
    U8 result = 1;
    U16 err;

    static U8 aesRef[A71CH_SYM_KEY_COMBINED_MAX][16] = {
        {0xDB, 0xFE, 0xE9, 0xE3, 0xB2, 0x76, 0x15, 0x4D, 0x67, 0xF9, 0xD8, 0x4C, 0xB9, 0x35, 0x54, 0x56},
        {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f},
        {0xC0, 0x79, 0xEF, 0x82, 0xCD, 0xF7, 0x12, 0xF2, 0x87, 0x28, 0xFD, 0x18, 0xED, 0xD7, 0xF2, 0xE4},
        {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x06, 0x07, 0x08, 0x09, 0xA0, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4},
    };

    // aesRef keys wrapped with themselves
    static U8 aesRefWrapped[A71CH_SYM_KEY_COMBINED_MAX][24] = {
        {0x83, 0x49, 0x96, 0xFE, 0x38, 0xD4, 0xBC, 0x97, 0x85, 0xA2, 0xC5, 0x1F, 0x9F, 0xD6, 0x4E, 0xF3,
         0x9C, 0x2C, 0x5A, 0xE0, 0xF2, 0x82, 0x02, 0x3F},
        {0x93, 0x5A, 0x3E, 0xB1, 0x01, 0xC3, 0x4A, 0xDD, 0x02, 0x5E, 0x17, 0x0B, 0x46, 0xFF, 0x0D, 0xB2,
         0x3E, 0x5C, 0x2F, 0xAE, 0x8C, 0x8F, 0x83, 0x70},
        {0xD0, 0x7B, 0xEB, 0x03, 0xE3, 0xB0, 0x01, 0x4F, 0xD9, 0x7D, 0x42, 0x3B, 0x9E, 0x51, 0x23, 0xEE,
         0x9A, 0x4B, 0x49, 0xAB, 0x9F, 0x10, 0xBE, 0x18},
        {0x2D, 0x36, 0x31, 0xEA, 0xF4, 0x69, 0xAD, 0xAF, 0x08, 0xE5, 0xB5, 0xEE, 0x97, 0xF5, 0x0A, 0x14,
         0xC2, 0x6A, 0x6F, 0xE8, 0xDC, 0xBB, 0x39, 0xF3}
    };

    // derivedDataRef used as reference
    static U8 derivedDataRef[A71CH_SYM_KEY_COMBINED_MAX][32] = {
        {0x36, 0x54, 0xBE, 0x49, 0x32, 0xB6, 0xE3, 0x84, 0xB8, 0x0C, 0xF5, 0xFC, 0xF7, 0x7B, 0x93, 0x93,
         0x82, 0x59, 0xD0, 0x45, 0x7E, 0xED, 0xE3, 0xB8, 0x6F, 0xC0, 0x91, 0x99, 0x67, 0x10, 0xA4, 0xF3},
        {0x9E, 0xA8, 0xD4, 0x14, 0xE2, 0x9A, 0x9A, 0x42, 0x2B, 0x45, 0x65, 0x21, 0xA1, 0x08, 0xA9, 0x85,
         0x97, 0x52, 0x03, 0xC3, 0x15, 0x23, 0xC0, 0xE2, 0x05, 0x86, 0x40, 0x8C, 0xE3, 0xAF, 0x39, 0x7F},
        {0xE8, 0x50, 0xE2, 0x1A, 0xB3, 0xBB, 0x41, 0xC0, 0x79, 0x39, 0x80, 0x1D, 0xA5, 0x87, 0x2F, 0x4C,
         0x4D, 0xE8, 0xB4, 0x6C, 0x2D, 0x56, 0x88, 0xA4, 0x44, 0x21, 0x65, 0xD6, 0xEB, 0xD8, 0x83, 0x3E},
        {0x2D, 0xCA, 0xE0, 0x37, 0x32, 0x7D, 0xA9, 0x97, 0xC6, 0x50, 0xAC, 0x9B, 0x93, 0x1E, 0x6F, 0x46,
         0xB6, 0xB5, 0x2B, 0xF6, 0x36, 0x57, 0xB2, 0x65, 0x70, 0x04, 0x5A, 0x06, 0x26, 0x37, 0x77, 0x2E}
    };

    U8 aesRefIdxLastWrappedByIdx0[] = {
         0x3C, 0x0B, 0x6E, 0x87, 0x11, 0x5C, 0xD1, 0x44, 0x34, 0x36, 0x25, 0x13, 0x6D, 0x66, 0xE5, 0x33,
         0x3D, 0x24, 0x7A, 0x42, 0xC8, 0x13, 0x38, 0xC1};
    U16 aesRefIdxLastWrappedByIdx0Len = sizeof(aesRefIdxLastWrappedByIdx0);

    U8 indexAesKey = 0;
    U8 indexWrapKey = 0;
    U8 indexWriteKey = 0;

    U8 info[] = {0x01, 0x02, 0x03, 0x04};
    U16 infoLen = sizeof(info);
    U8 derivedData[32];
    U16 derivedDataLen  = sizeof(derivedData);
    U8 nBlock = 0x01;

    PRINTF("\r\n-----------\r\nStart exAesRfc3394Precooked(%s)\r\n------------\r\n", getInitModeAsString(initMode));

    // Initialize the A71CH (Debug mode restrictions may apply)
    result &= a71chInitModule(initMode);

    for (indexAesKey=0; indexAesKey<A71CH_SYM_KEY_COMBINED_MAX; indexAesKey++)
    {
        // Write the key (unwrapped)
        PRINTF( "\r\nA71_SetSymKey(0x%02x)\r\n", indexAesKey);
        err = A71_SetSymKey((SST_Index_t)indexAesKey, aesRef[indexAesKey], sizeof(aesRef[indexAesKey]));
        result &= AX_CHECK_SW(err, SW_OK, "err");
        axPrintByteArray("aesRef[indexAesKey]", aesRef[indexAesKey], sizeof(aesRef[indexAesKey]), AX_COLON_32);
    }

    // To demonstrate the slots are filled up with keys, do a KDF
    for (indexAesKey=0; indexAesKey<A71CH_SYM_KEY_COMBINED_MAX; indexAesKey++)
    {
        PRINTF( "\r\nA71_HkdfExpandSymKey(0x%02x)\r\n", indexAesKey);
        derivedDataLen = sizeof(derivedData);
        err = A71_HkdfExpandSymKey((SST_Index_t)indexAesKey, nBlock, info, infoLen, derivedData, derivedDataLen);
        result &= AX_CHECK_SW(err, SW_OK, "err");
        axPrintByteArray("derivedData", derivedData, derivedDataLen, AX_COLON_32);
        result &= AX_COMPARE_BYTE_ARRAY("derivedDataRef[indexAesKey]", derivedDataRef[indexAesKey], sizeof(derivedDataRef[indexAesKey]),
            "derivedData", derivedData, derivedDataLen, AX_COLON_32);
    }

    // Inserting a wrapped key must succeed (inserting the same key value)
    for (indexAesKey=0; indexAesKey<A71CH_SYM_KEY_COMBINED_MAX; indexAesKey++)
    {
        PRINTF("\r\nA71_SetRfc3394WrappedAesKey(0x%02X)\r\n", indexAesKey);
        err = A71_SetRfc3394WrappedAesKey(indexAesKey, aesRefWrapped[indexAesKey], 24);
        result &= AX_CHECK_SW(err, SW_OK, "err");
        assert(result);
    }

    // To demonstrate the slots are filled up with the same keys, do a KDF
    for (indexAesKey=0; indexAesKey<A71CH_SYM_KEY_COMBINED_MAX; indexAesKey++)
    {
        PRINTF( "\r\nA71_HkdfExpandSymKey(0x%02x)\r\n", indexAesKey);
        derivedDataLen = sizeof(derivedData);
        err = A71_HkdfExpandSymKey((SST_Index_t)indexAesKey, nBlock, info, infoLen, derivedData, derivedDataLen);
        result &= AX_CHECK_SW(err, SW_OK, "err");
        // axPrintByteArray("derivedData", derivedData, derivedDataLen, AX_COLON_32);
        result &= AX_COMPARE_BYTE_ARRAY("derivedDataRef[indexAesKey]", derivedDataRef[indexAesKey], sizeof(derivedDataRef[indexAesKey]),
            "derivedData", derivedData, derivedDataLen, AX_COLON_32);
    }

    // Now replace the key at index 0, with the value of the key at index A71CH_SYM_KEY_COMBINED_MAX-1
    indexWriteKey = A71CH_SYM_KEY_0;
    indexWrapKey = A71CH_SYM_KEY_0;
    PRINTF("\r\nA71_SetRfc3394WrappedAesKey(0x%02X, wrapId=0x%02X)\r\n", indexWriteKey, indexWrapKey);
    err = A71_SetRfc3394WrappedAesKey((SST_Index_t)indexWriteKey, aesRefIdxLastWrappedByIdx0, aesRefIdxLastWrappedByIdx0Len);
    result &= AX_CHECK_SW(err, SW_OK, "err");
    assert(result);

    // Now do a KDF on the newly stored key at index A71CH_SYM_KEY_0
    // The resulting derived data must match the reference data stored on index A71CH_SYM_KEY_COMBINED_MAX-1
    indexAesKey = indexWriteKey;
    PRINTF( "\r\nA71_HkdfExpandSymKey(0x%02x)\r\n", indexAesKey);
    derivedDataLen = sizeof(derivedData);
    err = A71_HkdfExpandSymKey((SST_Index_t)indexAesKey, nBlock, info, infoLen, derivedData, derivedDataLen);
    result &= AX_CHECK_SW(err, SW_OK, "err");
    axPrintByteArray("derivedData", derivedData, derivedDataLen, AX_COLON_32);
    result &= AX_COMPARE_BYTE_ARRAY("derivedDataRef[A71CH_SYM_KEY_COMBINED_MAX-1]", derivedDataRef[A71CH_SYM_KEY_COMBINED_MAX-1], sizeof(derivedDataRef[A71CH_SYM_KEY_COMBINED_MAX-1]),
        "derivedData", derivedData, derivedDataLen, AX_COLON_32);

    PRINTF( "\r\n-----------\r\nEnd exAesRfc3394Precooked(), result = %s\r\n------------\r\n", ((result == 1)? "OK": "FAILED"));

    return result;
}
