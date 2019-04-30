/**
 * @file smComSocket_linux.c
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
 *
 */

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <time.h>
#include <netdb.h>

#include "smCom.h"
#include "smComSocket.h"
#include "sm_printf.h"

// Enable define of LOG_SOCK to echo APDU cmd/rsp
// #define LOG_SOCK

// Enable define of CHECK_ON_ATR to enable check on returned ATR (don't enable this when using the Smart Card Server ...)
#define CHECK_ON_ATR

#define REMOTE_JC_SHELL_HEADER_LEN             (4)
#define REMOTE_JC_SHELL_MSG_TYPE_APDU_DATA  (0x01)

#include "sm_apdu.h"
#define MAX_BUF_SIZE                (MAX_APDU_BUF_LENGTH)

typedef struct
{
    int sockfd;
    char * ipString;
} socket_Context_t;

static U8 Header[2] = {0x01,0x00};
static U8 sockapdu[MAX_BUF_SIZE];
static U8 response[MAX_BUF_SIZE];
static U8 *pCmd = (U8*) &sockapdu;
static U8 *pRsp = (U8*) &response;

static socket_Context_t sockCtx;
static socket_Context_t* pSockCtx = (socket_Context_t *)&sockCtx;

static U32 smComSocket_GetATR(U8* pAtr, U16* atrLen);

U16 smComSocket_Close()
{
    close(pSockCtx->sockfd);
    return SW_OK;
}

U16 smComSocket_Open(U8 *pIpAddrString, U16 portNo, U8* pAtr, U16* atrLen)
{
    int portno;
    int nAtr = 0;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    U16 sw = SMCOM_OK;

    portno = portNo;
    pSockCtx->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (pSockCtx->sockfd < 0)
    {
        printf("ERROR opening socket");
        return SMCOM_COM_FAILED;
    }

    pSockCtx->ipString = malloc(strlen((char*)pIpAddrString)+1);
    strcpy(pSockCtx->ipString, (char*)pIpAddrString);

    server = gethostbyname(pSockCtx->ipString);
    if (server == NULL)
    {
        fprintf(stderr,"ERROR, no such host: %s\r\n", pSockCtx->ipString);
        return SMCOM_COM_FAILED;
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(pSockCtx->sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    {
        printf("ERROR connecting\r\n");
        return SMCOM_PROTOCOL_FAILED;
    }

    smCom_Init(smComSocket_Transceive, smComSocket_TransceiveRaw);

    nAtr = smComSocket_GetATR(pAtr, atrLen);
#ifdef CHECK_ON_ATR
    // Be aware that the smart card server (java app on PC) does not return the ATR value
    // Do not enable this code when using the smart card server
    if (nAtr == 0)
    {
        sw = SMCOM_NO_ATR;
    }
#endif

    return sw;
}

/**
    Remote JC Terminal spec:
    Wait for card (MTY=0x00)
    The payload contains four bytes denoting the time in milliseconds the remote part will wait for card insertion.
    The bytes are sent in big endian format.

    The reply message contains the full ATR as payload.
    A reply message with 0 bytes length means that the terminal could not trigger an ATR (reason might be retrieved using MTY=3 or MTY=2.
*/
static U32 smComSocket_GetATR(U8* pAtr, U16* atrLen)
{
#define MTY 0
// #define NAD 0x21
#define NAD 0x00

    int retval;
#if defined(LOG_SOCK) || defined(DBG_LOG_SOCK)
    int i;
#endif
    U32 expectedLength = 0;
    U32 totalReceived = 0;
    U8 lengthReceived = 0;

    // wait 256 ms
    U8 ATRCmd[8] = {MTY, NAD, 0, 4, 0, 0, 1, 0};

#ifdef LOG_SOCK
    sm_printf(CONSOLE, "   send: ATR\r\n");
    for (i=0; i < sizeof(ATRCmd); i++)
    {
       sm_printf(CONSOLE, "%02X", ATRCmd[i]);
    }
    sm_printf(CONSOLE, "\r\n");
#endif

    retval = send(pSockCtx->sockfd, (const char*) ATRCmd, sizeof(ATRCmd), 0);
    if (retval < 0)
    {
       fprintf(stderr,"Client: send() failed: error %i.\r\n", retval);
       return 0;
    }

    expectedLength = REMOTE_JC_SHELL_HEADER_LEN; // remote JC shell header length

    while (totalReceived < expectedLength)
    {
        U32 maxCommLength;
        if (lengthReceived == 0)
        {
            maxCommLength = REMOTE_JC_SHELL_HEADER_LEN - totalReceived;
        }
        else
        {
            maxCommLength = expectedLength - totalReceived;
        }

        retval = recv(pSockCtx->sockfd, (char*) &pAtr[totalReceived], maxCommLength, 0);
        if (retval < 0)
        {
           fprintf(stderr,"Client: recv() failed: error %i.\r\n", retval);
           close(pSockCtx->sockfd);
           assert(0);
           return 0;
        }
        else
        {
            totalReceived += retval;
        }
        if ((totalReceived >= REMOTE_JC_SHELL_HEADER_LEN) && (lengthReceived == 0))
        {
            expectedLength += ((pAtr[2]<<8) | (pAtr[3]));
            lengthReceived = 1;
        }
    }
    retval = totalReceived;

#ifdef LOG_SOCK
    sm_printf(CONSOLE, "   full recv: ");
    for (i=0; i < retval; i++)
    {
       sm_printf(CONSOLE, "%02X", pAtr[i]);
    }
    sm_printf(CONSOLE, "\r\n");
#endif

    retval -= 4; // Remove the 4 bytes of the Remote JC Terminal protocol
    memmove(pAtr, pAtr + 4, retval);

#ifdef LOG_SOCK
    sm_printf(CONSOLE, "   recv: ");
    for (i=0; i < retval; i++)
    {
       sm_printf(CONSOLE, "%02X", pAtr[i]);
    }
    sm_printf(CONSOLE, "\r\n");
#endif

    *atrLen = (U16) retval;
    return retval;
}

U32 smComSocket_Transceive(apdu_t * pApdu)
{
    int retval;
#if defined(LOG_SOCK)
    int i;
#endif
    U32 txLen = 0;
    U32 expectedLength = 0;
    U32 totalReceived = 0;
    U8 lengthReceived = 0;

    assert(pApdu != NULL);

    pApdu->rxlen = 0;
   // TODO (?): adjustments on Le and Lc for SCP still to be done
    memset(sockapdu, 0x00, MAX_BUF_SIZE);
    memset(response, 0x00, MAX_BUF_SIZE);

    // remote JC Terminal header construction
    txLen = pApdu->buflen;
    memcpy(pCmd, Header, sizeof(Header));
    pCmd[2] = (txLen& 0xFF00)>>8;
    pCmd[3] = txLen & 0xFF;
    memcpy(&pCmd[4], pApdu->pBuf, pApdu->buflen);
    pApdu->buflen += 4; /* header & length */

#ifdef LOG_SOCK
    sm_printf(CONSOLE, "   send: ");
    for (i=4; i < (txLen+4); i++)
    {
       sm_printf(CONSOLE, "%02X", pCmd[i]);
    }
    sm_printf(CONSOLE, "\r\n");
#endif

    retval = send(pSockCtx->sockfd, (const char*) pCmd, pApdu->buflen, 0);
    if (retval < 0)
    {
        fprintf(stderr,"Client: send() failed: error %i.\r\n", retval);
        return SMCOM_SND_FAILED;
    }

    expectedLength = REMOTE_JC_SHELL_HEADER_LEN; // remote JC shell header length

    while (totalReceived < expectedLength)
    {
        retval = recv(pSockCtx->sockfd, (char*) &pRsp[totalReceived], MAX_BUF_SIZE, 0);
        if (retval < 0)
        {
           fprintf(stderr,"Client: recv() failed: error %i.\r\n", retval);
           close(pSockCtx->sockfd);
           assert(0);
           return SMCOM_RCV_FAILED;
        }
        else
        {
            totalReceived += retval;
        }
        if ((totalReceived >= REMOTE_JC_SHELL_HEADER_LEN) && (lengthReceived == 0))
        {
            expectedLength += ((pRsp[2]<<8) | (pRsp[3]));
            lengthReceived = 1;
        }
    }
    retval = totalReceived;

    retval -= 4; // Remove the 4 bytes of the Remote JC Terminal protocol
    memcpy(pApdu->pBuf, &pRsp[4], retval);

#ifdef LOG_SOCK
    sm_printf(CONSOLE, "   recv: ");
    for (i=0; i < retval; i++)
    {
       sm_printf(CONSOLE, "%02X", pApdu->pBuf[i]);
    }
    sm_printf(CONSOLE, "\r\n");
#endif

    pApdu->rxlen = (U16) retval;
    // reset offset for subsequent response parsing
    pApdu->offset = 0;
    return SMCOM_OK;
}

U32 smComSocket_TransceiveRaw(U8 * pTx, U16 txLen, U8 * pRx, U32 * pRxLen)
{
    S32 retval;
    U32 answerReceived = 0;
    U32 len = 0;
#if defined(LOG_SOCK) || defined(DBG_LOG_SOCK)
    int i;
#endif
    U32 readOffset = 0;
    U8 headerParsed = 0;
    U8 correctHeader = 0;
    memset(sockapdu, 0x00, MAX_BUF_SIZE);
    memset(response, 0x00, MAX_BUF_SIZE);

    memcpy(pCmd, Header, 2);
    pCmd[2] = (txLen & 0xFF00)>>8;
    pCmd[3] = (txLen & 0x00FF);
    memcpy(&pCmd[4], pTx, txLen);
    txLen += 4; /* header + len */

#ifdef DBG_LOG_SOCK
    sm_printf(CONSOLE, "   full send: ");
    for (i=0; i < txLen; i++)
    {
        sm_printf(CONSOLE, "%02X", pCmd[i]);
    }
    sm_printf(CONSOLE, "\r\n");
#endif

    retval = send(pSockCtx->sockfd, (const char*) pCmd, txLen, 0);
    if (retval < 0)
    {
        sm_printf(CONSOLE, "Client: send() failed: error %i.\r\n", retval);
        return SMCOM_SND_FAILED;
    }
    else
    {
#ifdef DBG_LOG_SOCK
        sm_printf(CONSOLE, "Client: send() is OK.\r\n");
#endif
    }

#ifdef LOG_SOCK
    sm_printf(CONSOLE, "   send: ");
    for (i=4; i < txLen; i++)
    {
        sm_printf(CONSOLE, "%02X", pCmd[i]);
    }
    sm_printf(CONSOLE, "\r\n");
#endif

    retval = REMOTE_JC_SHELL_HEADER_LEN; // receive at least the JCTerminal header

    while ((retval > 0) || (answerReceived == 0))
    {
        retval = recv(pSockCtx->sockfd, (char*) pRsp, MAX_BUF_SIZE, 0);

        if (retval < 0)
        {
           fprintf(stderr,"Client: recv() failed: error %i\r\n", retval);

           close(pSockCtx->sockfd);
           return SMCOM_RCV_FAILED;
        }
        else // data received
        {
            while (retval > 0) // parse all bytes
            {
                if (headerParsed == 1) // header already parsed; get data
                {
                    if (retval >= (S32) len)
                    {
                        if (correctHeader == 1)
                        {
                            memcpy(&pRx[0], &pRsp[readOffset], len);
                            answerReceived = 1;
                        }
                        else
                        {
                            // reset header parsed
                            readOffset += len;
                            headerParsed = 0;
                        }
                        retval -= len;

                        if (retval == 0) // no data left, reset readOffset
                        {
                            readOffset = 0;
                        }
                    }
                    else
                    {
                        // data too small according header => Error
                        fprintf(stderr,"Failed reading data %x %x\r\n", retval, len);
                        return SMCOM_RCV_FAILED;
                    }
                }
                else // parse header
                {
                    len = ((pRsp[readOffset + 2]<<8) | (pRsp[readOffset + 3]));

                    if (pRsp[readOffset] == REMOTE_JC_SHELL_MSG_TYPE_APDU_DATA)
                    {
                        // type correct => copy the data
                        retval -= REMOTE_JC_SHELL_HEADER_LEN;
                        if (retval > 0) // data left to read
                        {
                            readOffset += REMOTE_JC_SHELL_HEADER_LEN;
                        }
                        correctHeader = 1;
                    }
                    else
                    {
                        // type incorrect => skip the data as well and try again if data are left
                        readOffset += REMOTE_JC_SHELL_HEADER_LEN;
                        retval -= REMOTE_JC_SHELL_HEADER_LEN;
                        correctHeader = 0;
                    }
                    headerParsed = 1;
                }
            }
        }
    }

#ifdef LOG_SOCK
    sm_printf(CONSOLE, "   recv: ");
    for (i=0; i < len; i++)
    {
       sm_printf(CONSOLE, "%02X", pRx[i]);
    }
    sm_printf(CONSOLE, "\r\n");
#endif

    *pRxLen = len;

    return SMCOM_OK;
}
