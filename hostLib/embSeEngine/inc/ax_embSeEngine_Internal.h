/**
 * @file ax_embSeEngine_Internal.h
 * @author NXP Semiconductors
 * @version 1.0
 * @par License
 * Copyright 2017 NXP
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
 * OpenSSL Engine for Embedded Secure Element (A70CM/CI, A71CH)
 * Definitions and types with local scope
 */

#ifndef AX_EMB_SE_ENGINE_INTERNAL_H
#define AX_EMB_SE_ENGINE_INTERNAL_H

#include "ax_api.h"

#ifdef __cplusplus
extern "C" {
#endif

// Looking for a key reference in a key object can lead to either of the following results
#define AX_ENGINE_INVOKE_NOTHING     0 // Do no nothing, key object is not valid
#define AX_ENGINE_INVOKE_SE          1 // Found a reference to a key contained in the Secure Element
#define AX_ENGINE_INVOKE_OPENSSL_SW  2 // Pass on key object to OpenSSL SW implementation

#if defined(TGT_A71CH)
typedef U8 SST_Identifier_t;
#endif

typedef struct _axKeyIdentifier_t
{
    SST_Identifier_t ident;
    SST_Index_t idx;
} axKeyIdentifier_t;


#ifdef __cplusplus
}
#endif

#endif // AX_EMB_SE_ENGINE_INTERNAL_H
