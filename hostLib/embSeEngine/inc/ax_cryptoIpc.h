/*****************************************************************************
 * Copyright 2016 NXP
 *
 * This software is owned or controlled by NXP and may only be used
 * strictly in accordance with the applicable license terms.  By expressly
 * accepting such terms or by downloading, installing, activating and/or
 * otherwise using the software, you are agreeing that you have read, and
 * that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you
 * may not retain, install, activate or otherwise use the software.
 ****************************************************************************/

#ifndef _AX_CRYPTOIPC_H
#define _AX_CRYPTOIPC_H

#define AX_CI_TRUE  1
#define AX_CI_FALSE 0

#ifdef __cplusplus
extern "C" {
#endif

int CryptoIpc_MutexInit(int setval);
void CryptoIpc_MutexLock(void);
void CryptoIpc_MutexUnlock(void);
char* CryptoIpc_ShmInit(void);
int CryptoIpc_Close(void);

#ifdef __cplusplus
}
#endif

#endif // _AX_CRYPTOIPC_H
