/**
 * @file rjct.c
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
 * Connection Oriented TCP/IP Server implementing Remote JCTerminal Protocol.
 * The server can connect to the card via the
 * - TDA-UART protocol
 * - SCI2C
 * - PCSC
 * @par History
 *
 *****************************************************************************/
#include <stdio.h>
#include <string.h>

#include "rjct.h"

U16 rjctPackageApduResponse(U8 messageType, U8 nodeAddress, U8* payload, U16 payloadLen, U8 *targetBuf, U16 *targetBufLen)
{
    if (*targetBufLen < (4+payloadLen))
    {
        printf("Target buffer provided too small.\n");
        return RJCT_ARG_FAIL;
    }

    targetBuf[0] = messageType;
    targetBuf[1] = nodeAddress;
    targetBuf[2] = (payloadLen >> 8) & 0x00FF;
    targetBuf[3] = payloadLen & 0x00FF;
    memcpy(&targetBuf[4], payload, payloadLen);
    *targetBufLen = 4 + payloadLen;
    return RJCT_OK;
}
