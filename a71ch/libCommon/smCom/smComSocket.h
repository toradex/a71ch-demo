/**
 * @file smComSocket.h
 * @author NXP Semiconductors
 * @version 1.1
 * @par License
 * Copyright(C) NXP Semiconductors, 2016,2017
 * This software is owned or controlled by NXP and may only be used
 * strictly in accordance with the applicable license terms.  By expressly
 * accepting such terms or by downloading, installing, activating and/or
 * otherwise using the software, you are agreeing that you have read, and
 * that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you
 * may not retain, install, activate or otherwise use the software.
 *
 * @par Description
 *
 *****************************************************************************/

#ifndef _SCCOMSOCKET_H_
#define _SCCOMSOCKET_H_

#include "smCom.h"

#ifdef __cplusplus
extern "C" {
#endif

U16 smComSocket_Close();
U16 smComSocket_Open(U8 *pIpAddrString, U16 portNo, U8 *pAtr, U16 *atrLen);
#if defined(_WIN32) && defined(TGT_A70CU)
U16 smComSocket_Init(U8 *pIpAddrString, U16 portNo, U8 *pAtr, U16 *pAtrLength, U16 maxAtrLength);
#endif
U32 smComSocket_Transceive(apdu_t *pApdu);
U32 smComSocket_TransceiveRaw(U8 *pTx, U16 txLen, U8 *pRx, U32 *pRxLen);

#ifdef __cplusplus
}
#endif
#endif //_SCCOMSOCKET_H_
