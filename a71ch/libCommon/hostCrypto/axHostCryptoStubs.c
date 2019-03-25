/**
 * @file axHostCryptoStubs.c
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
 * Host Crypto stub implementation for the A7-series
 *
 * @par HISTORY
 *
 */

#include "axHostCrypto.h"
#include "ax_util.h"
#include "sm_types.h"
#include "sm_printf.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Provide here your own implementation (In case crypto is required and OpenSSL is not available)
S32 HOST_SHA1_Get(const U8 *msg, U32 msgLen, U8 *pHash)
{
    return HOST_CRYPTO_ERROR;
}

S32 HOST_SHA256_Get(const U8 *msg, U32 msgLen, U8 *pHash)
{
    return HOST_CRYPTO_ERROR;
}

S32 HOST_AES_ECB_DECRYPT(U8 *plainText, const U8 *cipherText, const U8 *decryptKey, U32 decryptKeyLen)
{
    return HOST_CRYPTO_ERROR;
}

S32 HOST_AES_ECB_ENCRYPT(const U8 *plainText, U8 *cipherText, const U8 *encryptKey, U32 encryptKeyLen)
{
    return HOST_CRYPTO_ERROR;
}

S32 HOST_CMAC_Get(const U8 *pKey, U8 keySizeInBytes, const U8* pMsg, U32 msgLen, U8 *pMac)
{
    return HOST_CRYPTO_ERROR;
}

S32 HOST_CMAC_Init(axHcCmacCtx_t **ctx, const U8 *pKey,  U8 keySizeInBytes)
{
    return HOST_CRYPTO_ERROR;
}

S32 HOST_CMAC_Update(axHcCmacCtx_t *ctx, const U8 *pMsg, U32 msgLen)
{
    return HOST_CRYPTO_ERROR;
}

S32 HOST_CMAC_Finish(axHcCmacCtx_t *ctx, U8 *pMac)
{
    return HOST_CRYPTO_ERROR;
}

S32 HOST_AES_CBC_Process(const U8 *pKey, U32 keyLen, const U8 *pIv, U8 dir, const U8 *pIn, U32 inLen, U8 *pOut)
{
    return HOST_CRYPTO_ERROR;
}

S32 HOST_GetRandom(U32 inLen, U8 *pRandom)
{
    return HOST_CRYPTO_ERROR;
}
