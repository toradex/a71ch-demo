/**
* @file rjct_com.c
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
* This file implements basic communication functionality between Host and
* Secure element.
* @par History
* 1.0   26-march-2014 : Initial version
*
*****************************************************************************/
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#include "rjct.h"

#ifdef LINUX
#ifdef TDA8029_UART
#include "smUart.h"
#include "smComAlpar.h"
#elif defined(I2C)
#include "smComSCI2C.h"
#elif defined(PCSC)
#include "smComPCSC.h"
#endif
#else
#include "smComSocket.h"
#endif


#include "global_platf.h"

#ifdef FLOW_VERBOSE
#define FPRINTF(...) printf (__VA_ARGS__)
#else
#define FPRINTF(...)
#endif

/**
 * SM_ConnectRjct
 * @param[in] commState
 * @param[out] atr
 * @param[in,out] atrLen
 * @return ::ERR_CONNECT_LINK_FAILED    No communication with TDA chip (and/or) Secure Module
 * @return ::SMCOM_COM_FAILED           Cannot open communication channel on the Host
 * @return ::SMCOM_PROTOCOL_FAILED      No communication with Secure Module
 * @return 0x9000                       OK
 */
U16 SM_ConnectRjct(SmCommStateRjct_t *commState, U8 *atr, U16 *atrLen)
{
    U16 sw = SW_OK;
    U16 uartBR = 0;
    U16 t1BR = 0;
#ifdef TDA8029_UART
    U32 status = 0;
#elif defined (I2C)
    U8 dummyAtr[64];
    U16 dummyAtrLen = sizeof(dummyAtr);
    U8 precookedI2cATR[] = {
        0x3B, 0xFB, 0x18, 0x00, 0x00, 0x81, 0x31, 0xFE, 0x45, 0x50, 0x4C, 0x41, 0x43, 0x45, 0x48, 0x4F, 0x4C,
        0x44, 0x45, 0x52, 0xAB};
#endif

    assert(commState!=NULL);

#ifdef TDA8029_UART
    assert(atr!=NULL);
    assert(atrLen!=NULL);
    assert((*atrLen)>33);

    smComAlpar_Init();
    status = smComAlpar_AtrT1Configure(ALPAR_T1_BAUDRATE_MAX, atr, atrLen, &uartBR, &t1BR);
    if (status != SMCOM_ALPAR_OK )
    {
        commState->param1 = 0;
        commState->param2 = 0;
        FPRINTF("smComAlpar_AtrT1Configure failed: 0x%08X\n", status);
        return ERR_CONNECT_LINK_FAILED;
    }
#elif defined(I2C)
    // The smComSCI2C_Open function returns an SCI2C compliant ATR value.
    // This value can not be used as is as ATR parameter to the SM_Connect function because it is
    // not ISO7816-3 compliant. Instead a pre-cooked value is used.
    // In case no SCI2C ATR can be retrieved by smComSCI2C_Open, no Secure Element is attached.
    sw = smComSCI2C_Open(ESTABLISH_SCI2C, 0x00, dummyAtr, &dummyAtrLen);
#elif defined(PCSC)
    sw = smComPCSC_Open(0, atr, atrLen);
#endif
    commState->param1 = t1BR;
    commState->param2 = uartBR;
#if defined(I2C)
    if (sw == SW_OK)
    {
        if (dummyAtrLen == 0)
        {
            FPRINTF("smComSCI2C_Open failed. No secure module attached");
            *atrLen = 0;
            return ERR_CONNECT_LINK_FAILED;
        }
        else
        {
            int i = 0;
            FPRINTF("SCI2C_ATR=0x");
            for (i=0; i<dummyAtrLen; i++) FPRINTF("%02X.", dummyAtr[i]);
            FPRINTF("\n");
        }

        memcpy(atr, precookedI2cATR, sizeof(precookedI2cATR));
        *atrLen = sizeof(precookedI2cATR);
    }
#endif

    return sw;
}


U16 SM_SendAPDURjct(U8 *cmd, U16 cmdLen, U8 *resp, U16 *respLen)
{
    U32 status = 0;
    U32 respLenLocal = *respLen;

    status = smCom_TransceiveRaw(cmd, cmdLen, resp, &respLenLocal);
    *respLen = (U16)respLenLocal;

    return (U16) status;
}
