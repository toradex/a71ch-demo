/**
 * @file a71chTlsClient.c
 * @author NXP Semiconductors
 * @version 1.0
 * @par License
 * Copyright 2018 NXP
 *
 * This software is owned or controlled by NXP and may only be used
 * strictly in accordance with the applicable license terms.  By expressly
 * accepting such terms or by downloading, installing, activating and/or
 * otherwise using the software, you are agreeing that you have read, and
 * that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you
 * may not retain, install, activate or otherwise use the software.
 *
 * @par Description:
 * Simple TLS client
 *
 * Precondition:
 * - A71CH has been provisioned with appropriate credentials
 * - Server side has been setup and has been provisioned with appropriate credentials
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
// #include <dlfcn.h>
#include <fcntl.h>
#include <openssl/ssl.h>
#include <openssl/engine.h>

#include "sm_types.h"
#include "ax_api.h"
#include "HLSEAPI.h"

#define A71CH_TLS_CLIENT_OK                          0
#define A71CH_TLS_CLIENT_CMDLINE_ARGS                1
#define A71CH_TLS_CLIENT_BUFFER_TOO_SMALL            2
#define A71CH_TLS_CLIENT_ILLEGAL_PORT_NR             3

#define A71CH_TLS_CLIENT_TCP_SETUP_ERROR           100
#define A71CH_TLS_CLIENT_TCP_CONNECT_ERROR         101

#define A71CH_TLS_CLIENT_SSL_LIB_INIT_ERROR        200
#define A71CH_TLS_CLIENT_SSL_INIT_ERROR            201
#define A71CH_TLS_CLIENT_SSL_CERT_ERROR            202
#define A71CH_TLS_CLIENT_SSL_KEY_ERROR             203
#define A71CH_TLS_CLIENT_SSL_CONNECT_ERROR         204
#define A71CH_TLS_CLIENT_SSL_CONNECT_TIMEOUT_ERROR 205
#define A71CH_TLS_CLIENT_SSL_WRITE_TIMEOUT_ERROR   206
#define A71CH_TLS_CLIENT_SSL_WRITE_ERROR           207

#define A71CH_ERR_A71CH_GP_ACCESS                  300
#define A71CH_ERR_A71CH_GP_NO_CERT                 301
#define A71CH_ERR_A71CH_LOADING_ENGINE_ERR         302

/* App version */
const int v_major = 0;
const int v_minor = 8;
const int v_patch = 2;

static void printDataAsHex(U8* data, U16 dataLen);

#define OK_STATUS      0
#define FAILURE_STATUS 1

int certVerifyCb(int preverifyOk, X509_STORE_CTX *pX509CTX)
{
    char szData[256];
    X509 *cert = X509_STORE_CTX_get_current_cert(pX509CTX);
    int depth = X509_STORE_CTX_get_error_depth(pX509CTX);

    if (!preverifyOk) {
        int err = X509_STORE_CTX_get_error(pX509CTX);
        printf("Error with certificate at depth: %i\n", depth);
        X509_NAME_oneline(X509_get_issuer_name(cert), szData, 256);
        printf(" issuer = %s\n", szData);
        X509_NAME_oneline(X509_get_subject_name(cert), szData, 256);
        printf(" subject = %s\n", szData);
        printf(" err %i:%s\n", err, X509_verify_cert_error_string(err));
    }
    else {
        printf("Certificate at depth: %i\n", depth);
        X509_NAME_oneline(X509_get_issuer_name(cert), szData, 256);
        printf(" issuer = %s\n", szData);
        X509_NAME_oneline(X509_get_subject_name(cert), szData, 256);
        printf(" subject = %s\n", szData);
    }
    return preverifyOk;
}

static int tcpConnectWrapper(int socket_fd, char *pURLString, int port)
{
    int nRet = A71CH_TLS_CLIENT_TCP_CONNECT_ERROR;
    int connect_status = -1;
    struct hostent *host;
    struct sockaddr_in dest_addr;

    host = gethostbyname(pURLString);

    if (NULL != host) {
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(port);
        dest_addr.sin_addr.s_addr = *(long*) (host->h_addr);
        memset(&(dest_addr.sin_zero), '\0', 8);

        connect_status = connect(socket_fd, (struct sockaddr *) &dest_addr,
                sizeof(struct sockaddr));
        if (connect_status != -1) {
            nRet = A71CH_TLS_CLIENT_OK;
        }
    }
    return nRet;
}

static int setSocketNonBlockingWrapper(int server_fd)
{
    int flags, status;
    int nRet = A71CH_TLS_CLIENT_OK;

    flags = fcntl(server_fd, F_GETFL, 0);
    // set underlying socket to non blocking
    if (flags < 0) {
        nRet = A71CH_TLS_CLIENT_TCP_CONNECT_ERROR;
    }

    status = fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);
    if (status < 0) {
        printf("fcntl - %s", strerror(errno));
        nRet = A71CH_TLS_CLIENT_TCP_CONNECT_ERROR;
    }

    return nRet;
}

#define SSL_CONNECTED    1
#define SELECT_TIMEOUT   0
#define SELECT_ERROR    -1

static int sslConnectWrapper(SSL *pSSL, int server_fd, int timeout_ms)
{
    int nRet = A71CH_TLS_CLIENT_OK;
    int rc = 0;
    fd_set readFds;
    fd_set writeFds;
    struct timeval timeout = { timeout_ms / 1000, (timeout_ms % 1000) * 1000 };
    int errorCode = 0;
    int nSelect = SELECT_TIMEOUT;

    do {
        printf("Invoke SSL_connect()...\n");
        rc = SSL_connect(pSSL);

        if (SSL_CONNECTED == rc) {
            nRet = A71CH_TLS_CLIENT_OK;
            break;
        }

        errorCode = SSL_get_error(pSSL, rc);

        if (errorCode == SSL_ERROR_WANT_READ) {
            FD_ZERO(&readFds);
            FD_SET(server_fd, &readFds);
            nSelect = select(server_fd + 1, (void *) &readFds, NULL, NULL, &timeout);
            if (SELECT_TIMEOUT == nSelect) {
                printf("sslConnectWrapper: time out while waiting for read.\n");
                nRet = A71CH_TLS_CLIENT_SSL_CONNECT_TIMEOUT_ERROR;
            }
            else if (SELECT_ERROR == nSelect) {
                printf("sslConnectWrapper: select error for read %d.\n", nSelect);
                nRet = A71CH_TLS_CLIENT_SSL_CONNECT_ERROR;
            }
        }
        else if (errorCode == SSL_ERROR_WANT_WRITE) {
            FD_ZERO(&writeFds);
            FD_SET(server_fd, &writeFds);
            nSelect = select(server_fd + 1, NULL, (void *) &writeFds, NULL, &timeout);
            if (SELECT_TIMEOUT == nSelect) {
                printf("sslConnectWrapper: time out while waiting for write.\n");
                nRet = A71CH_TLS_CLIENT_SSL_CONNECT_TIMEOUT_ERROR;
            }
            else if (SELECT_ERROR == nSelect) {
                printf("sslConnectWrapper: select error for write %d.\n", nSelect);
                nRet = A71CH_TLS_CLIENT_SSL_CONNECT_ERROR;
            }
        }
        else {
            nRet = A71CH_TLS_CLIENT_SSL_CONNECT_ERROR;
        }

    } while ( (A71CH_TLS_CLIENT_SSL_CONNECT_ERROR != nRet) &&
              (A71CH_TLS_CLIENT_SSL_CONNECT_TIMEOUT_ERROR != nRet) );

    return nRet;
}


int sslWriteWrapper(SSL *pSSL, int server_fd, unsigned char *msg, int *totalLen, int timeout_ms)
{
    int nRet = A71CH_TLS_CLIENT_OK;
    fd_set writeFds;
    int errorCode = 0;
    int nSelect;
    int writtenLen = 0;
    int rc = 0;
    struct timeval timeout = { timeout_ms / 1000, (timeout_ms % 1000) * 1000 };

    do {
        rc = SSL_write(pSSL, msg, *totalLen);

        errorCode = SSL_get_error(pSSL, rc);

        if (0 < rc) {
            writtenLen += rc;
        }
        else if (errorCode == SSL_ERROR_WANT_WRITE) {
            FD_ZERO(&writeFds);
            FD_SET(server_fd, &writeFds);
            nSelect = select(server_fd + 1, NULL, (void *) &writeFds, NULL, &timeout);
            if (SELECT_TIMEOUT == nSelect) {
                nRet = A71CH_TLS_CLIENT_SSL_WRITE_TIMEOUT_ERROR;
            }
            else if (SELECT_ERROR == nSelect) {
                nRet = A71CH_TLS_CLIENT_SSL_WRITE_ERROR;
            }
        }
        else {
            nRet = A71CH_TLS_CLIENT_SSL_WRITE_ERROR;
        }

    } while ( (A71CH_TLS_CLIENT_SSL_WRITE_ERROR != nRet) &&
              (A71CH_TLS_CLIENT_SSL_WRITE_TIMEOUT_ERROR != nRet) &&
              (writtenLen < *totalLen) );

    *totalLen = writtenLen;
    return nRet;
}

int wrapConnectToA71CH()
{
    U16 connectStatus = 0;
    U8 Atr[64];
    U16 AtrLen = sizeof(Atr);
    SmCommState_t commState;

#if defined(I2C) || defined(PCSC) || defined(SPI)
    connectStatus = SM_Connect(&commState, Atr, &AtrLen);
#elif defined(RJCT_SOCKET)
    connectStatus = SM_RjctConnect("192.168.2.70:8050", &commState, Atr, &AtrLen);
#else
    #error "No communication channel defined"
#endif // TDA8029
    if ( (connectStatus == ERR_CONNECT_LINK_FAILED) || (connectStatus == ERR_CONNECT_SELECT_FAILED) )
    {
        printf("SM_Connect failed with status 0x%04X\n", connectStatus);
        return 2;
    }
    else if ( connectStatus == SMCOM_COM_FAILED )
    {
        printf("SM_Connect failed with status 0x%04X (Could not open communication channel)\n", connectStatus);
        return 4;
    }
    else if ( connectStatus == SMCOM_PROTOCOL_FAILED)
    {
        printf("SM_Connect failed with status 0x%04X (Could not establish communication protocol)\n", connectStatus);
        return 5;
    }
    else if ( connectStatus == ERR_NO_VALID_IP_PORT_PATTERN )
    {
        printf("Pass the IP address and port number as arguments, e.g. \"127.0.0.1:8050\"!\n");
        return 3;
    }
    else
    {
        int i=0;
#if defined(I2C)
        printf("SCI2C_"); // To highlight the ATR format for SCI2C deviates from ISO7816-3
#elif defined(SPI)
        printf("SCSPI_");
#endif
        if (AtrLen > 0)
        {
            printf("ATR=0x");
            for (i=0; i<AtrLen; i++) { printf("%02X.", Atr[i]); }
            printf("\n");
        }
#if defined(TDA8029_UART)
        printf("UART Baudrate Idx: 0x%02X\n", commState.param2);
        printf("T=1           TA1: 0x%02X\n", commState.param1);
#endif
        printf("HostLib Version  : 0x%04X\n", commState.hostLibVersion);
        if (connectStatus != SW_OK)
        {
            printf("Select failed. SW = 0x%04X\n", connectStatus);
            return 5;
        }
        printf("Applet Version   : 0x%04X\n", commState.appletVersion);
        printf("SecureBox Version: 0x%04X\n", commState.sbVersion);
    }
    printf("========================\n");

    if (commState.appletVersion < 0x0130)
    {
        printf("Warning: Please switch to latest A71CH version, attached version is obsolete (attached version=0x%04X)\n", commState.appletVersion);
        return 6;
    }

    return A71CH_TLS_CLIENT_OK;
}

#define MAX_DATA_HANDLE 255
int a71GetDataObject(int index, U16 offset, U16 nByte, U8 *data, U16 *dataLen)
{
    int i;
    int handleWasSet = 0;
    HLSE_RET_CODE hlseRc;
    HLSE_ATTRIBUTE attr;
    HLSE_DIRECT_ACCESS_ATTRIBUTE_VALUE theValue;
    HLSE_OBJECT_HANDLE dataHandle[MAX_DATA_HANDLE];
    HLSE_OBJECT_HANDLE readHandle;
    U16 dataHandleNum = sizeof(dataHandle) / sizeof(HLSE_OBJECT_HANDLE);

    // Enumerate handles
    dataHandleNum = sizeof(dataHandle) / sizeof(HLSE_OBJECT_HANDLE);
    hlseRc = HLSE_EnumerateObjects(HLSE_DATA, dataHandle, &dataHandleNum);
    if (hlseRc != HLSE_SW_OK) { return A71CH_ERR_A71CH_GP_ACCESS; }

    // We are looking for the (certificate) object on certIndex;
    printf("getData on index %d\n", index);

    // Find handle
    for (i = 0; i < dataHandleNum; i++) {
        // printf("Looking at index %d: 0x%02X <> 0x%02X\n", i, (dataHandle[i] & 0xF), curHandle);
        if (HLSE_GET_OBJECT_INDEX(dataHandle[i]) == (U8)index) {
            readHandle = dataHandle[i];
            handleWasSet = 1;
            break;
        }
    }
    if (!handleWasSet) { return A71CH_ERR_A71CH_GP_NO_CERT; }

    // Read
    attr.type = HLSE_ATTR_DIRECT_ACCESS_OBJECT_VALUE;

    // Pre-cooked values for offset and chunk size to read
    theValue.offset = offset;
    theValue.bytes = nByte;
    theValue.buffer = data;
    theValue.bufferLen = *dataLen;

    attr.value = &theValue;
    attr.valueLen = sizeof(theValue);

    hlseRc = HLSE_GetObjectAttribute(readHandle, &attr);
    if (hlseRc != HLSE_SW_OK) {
        return A71CH_ERR_A71CH_GP_NO_CERT;
    }

    *dataLen = theValue.bytes;
    printf("Size of data object retrieved=%d\n", *dataLen);
    printDataAsHex(data, *dataLen);

    return A71CH_TLS_CLIENT_OK;
}

#define MAX_CERT_HANDLE 255
int a71GetClientCertificate(int certIndex, U8 *clientCertDer, U16 *clientCertDerLen)
{
    int i;
    int handleWasSet = 0;
    HLSE_RET_CODE hlseRc;
    HLSE_ATTRIBUTE attr;
    HLSE_OBJECT_HANDLE certHandles[MAX_CERT_HANDLE];
    HLSE_OBJECT_HANDLE readHandle;
    U16 certHandlesNum = sizeof(certHandles) / sizeof(HLSE_OBJECT_HANDLE);

    // Enumerate handles
    certHandlesNum = sizeof(certHandles) / sizeof(HLSE_OBJECT_HANDLE);
    hlseRc = HLSE_EnumerateObjects(HLSE_CERTIFICATE, certHandles, &certHandlesNum);
    if (hlseRc != HLSE_SW_OK) { return A71CH_ERR_A71CH_GP_ACCESS; }

    // We are looking for the (certificate) object on certIndex;
    printf("getCertificate on index %d\n", certIndex);

    // Find handle
    for (i = 0; i < certHandlesNum; i++) {
        // printf("Looking at index %d: 0x%02X <> 0x%02X\n", i, (certHandles[i] & 0xF), curHandle);
        if (HLSE_GET_OBJECT_INDEX(certHandles[i]) == (U8)certIndex) {
            readHandle = certHandles[i];
            handleWasSet = 1;
            break;
        }
    }
    if (!handleWasSet) { return A71CH_ERR_A71CH_GP_NO_CERT; }

    // Read
    attr.type = HLSE_ATTR_OBJECT_VALUE;
    attr.value = clientCertDer;
    attr.valueLen = *clientCertDerLen;
    hlseRc = HLSE_GetObjectAttribute(readHandle, &attr);
    if (hlseRc != HLSE_SW_OK) {
        return A71CH_ERR_A71CH_GP_NO_CERT;
    }

    *clientCertDerLen = attr.valueLen;
    printf("Size of certificate retrieved=%d\n", *clientCertDerLen);
    // printDataAsHex(clientCertDer, *clientCertDerLen);

    return A71CH_TLS_CLIENT_OK;
}

#define MAX_DER_CERT_SIZE    1024
#define MAX_DATA_OBJECT_SIZE 1024

int main(int argc, char **argv)
{
    int nStatus = 0;
    ENGINE *e;
    const SSL_METHOD *method;
    SSL_CTX *pSSLContext;
    SSL *pSSLHandle;
    char pDestinationURL[256];
    int nDestinationPort = 8080;
    int nRet = A71CH_TLS_CLIENT_OK;
    int sockfd;
    char fileRootCA[256];
    char fileClientKey[256];
    X509 *certRcv;
    char clientMessage[256];
    int clientMessageLen;
    U8 clientCerDer[MAX_DER_CERT_SIZE];
    U16 clientCerDerLen = MAX_DER_CERT_SIZE;
    int certIndex = 0;
    U8 dataObject[MAX_DATA_OBJECT_SIZE];
    U16 dataObjectLen = MAX_DATA_OBJECT_SIZE;
    int objectIndex = 0;
    U16 dataOffset;
    U16 dataLen;

    printf("a71chTlsClient (Rev.%d.%d.%d)\n", v_major, v_minor, v_patch);

    // Evaluate command line arguments
    if (argc != 4) {
        printf("Usage: a71chTlsClient <ipAddress:port> <caCert.pem> <clientKey.pem|clientKeyRef.pem>\n");
        nRet = A71CH_TLS_CLIENT_CMDLINE_ARGS;
        goto leaving;
    }

    // Deal with first argument: <address|name>:<port>
    // ***********************************************
    {
        unsigned int i;
        int fColonFound = 0;
        int nSuccess;

        if (strlen(argv[1]) >= sizeof(pDestinationURL)) {
            // A bit too cautious (as port number may also be attached)
            nRet = A71CH_TLS_CLIENT_BUFFER_TOO_SMALL;
            goto leaving;
        }

        for (i = 0; i < strlen(argv[1]); i++) {
            if (argv[1][i] == ':') {
                strncpy(pDestinationURL, argv[1], i);
                pDestinationURL[i] = 0;
                fColonFound = 1;
                // printf("servername/address: %s\n", pDestinationURL);
                break;
            }
            else {
                // Simply copy the full argument
                strcpy(pDestinationURL, argv[1]);
            }
        }

        if ((fColonFound == 1) && (i != 0)) {
            nSuccess = sscanf(&argv[1][i], ":%5u[0-9]", (unsigned int *)&nDestinationPort);
            if (nSuccess != 1) {
                nRet = A71CH_TLS_CLIENT_ILLEGAL_PORT_NR;
                goto leaving;
            }
        }
        else {
            // Choose default value for port
            nDestinationPort = 8080;
        }
    }
    printf("\t servername:port = %s:%d\n", pDestinationURL, nDestinationPort);

    // Deal with second argument: root CA certificate
    // **********************************************
    if (strlen(argv[2]) >= sizeof(fileRootCA)) {
        nRet = A71CH_TLS_CLIENT_BUFFER_TOO_SMALL;
        goto leaving;
    }
    strcpy(fileRootCA, argv[2]);
    printf("\t rootCA: %s\n", fileRootCA);

    // Deal with third argument: key or reference key
    // **********************************************
    if (strlen(argv[3]) >= sizeof(fileClientKey)) {
        nRet = A71CH_TLS_CLIENT_BUFFER_TOO_SMALL;
        goto leaving;
    }
    strcpy(fileClientKey, argv[3]);
    printf("\t clientKey: %s\n", fileClientKey);

    strcpy(clientMessage, "Hello to Server from TLS client using credentials stored in A71CH.\n");
    clientMessageLen = strlen(clientMessage);

    nRet = wrapConnectToA71CH();
    if (nRet != A71CH_TLS_CLIENT_OK) {
        printf("Failed to connect to A71CH.\n");
        goto leaving;
    }

    certIndex = 0;
    nRet = a71GetClientCertificate(certIndex, clientCerDer, &clientCerDerLen);
    if (nRet != A71CH_TLS_CLIENT_OK) {
        printf("Failed to retrieve client certificate.\n");
        goto leaving;
    }

    SM_Close(SMCOM_CLOSE_MODE_STD);

    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    if (SSL_library_init() < 0) {
        nRet = A71CH_TLS_CLIENT_SSL_LIB_INIT_ERROR;
        goto leaving;
    }

    method = TLSv1_2_method();

    if ((pSSLContext = SSL_CTX_new(method)) == NULL) {
        printf("SSL_CTX_new failed - Unable to create SSL Context\n");
        nRet = A71CH_TLS_CLIENT_SSL_INIT_ERROR;
        goto leaving;
    }

    if (!(e = ENGINE_by_id("e2a71ch")))
    {
        fprintf(stderr, "Error finding OpenSSL Engine by id (id = %s)\n", "e2a71ch");
        nRet = A71CH_ERR_A71CH_LOADING_ENGINE_ERR;
        goto leaving;
    }
    else
    {
        unsigned int logLevel = 0x05;
        fprintf(stderr, "Setting log level OpenSSL-engine %s to 0x%02X.\n", "e2a71ch", logLevel);
        // Illustrates setting log level of e2a71ch engine.
        // 0x04 : Error message
        // 0x02 : Debug messages
        // 0x01 : Flow messages
        // (0x07 corresponds to all, which is the default)
        ENGINE_ctrl(e, ENGINE_CMD_BASE, logLevel, NULL, NULL);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        nRet = A71CH_TLS_CLIENT_TCP_SETUP_ERROR;
        goto leaving;
    }

    if (!SSL_CTX_load_verify_locations(pSSLContext, fileRootCA, NULL)) {
        nRet = A71CH_TLS_CLIENT_SSL_CERT_ERROR;
        goto leaving;
    }

    if (!SSL_CTX_use_certificate_ASN1(pSSLContext, clientCerDerLen, clientCerDer)) {
        printf("Client Certificate Loading error.\n");
        nRet = A71CH_TLS_CLIENT_SSL_CERT_ERROR;
        goto leaving;
    }

    if (SSL_CTX_use_PrivateKey_file(pSSLContext, fileClientKey, SSL_FILETYPE_PEM) != 1){
        printf("Client Private Key Loading error.\n");
        nRet = A71CH_TLS_CLIENT_SSL_KEY_ERROR;
        goto leaving;
    }

    SSL_CTX_set_verify(pSSLContext, SSL_VERIFY_PEER, certVerifyCb);
    // SSL_CTX_set_verify(pSSLContext, SSL_VERIFY_PEER, NULL);

    pSSLHandle = SSL_new(pSSLContext);

    nRet = tcpConnectWrapper(sockfd, pDestinationURL, nDestinationPort);
    if (nRet != A71CH_TLS_CLIENT_OK) {
        printf("TCP Connection error.\n");
        goto leaving;
    }

    SSL_set_fd(pSSLHandle, sockfd);

    nRet = setSocketNonBlockingWrapper(sockfd);
    if (nRet != A71CH_TLS_CLIENT_OK) {
        printf("Unable to set the socket to Non-Blocking.\n");
        goto leaving;
    }

    nRet = sslConnectWrapper(pSSLHandle, sockfd, 2000);
    if (nRet != A71CH_TLS_CLIENT_OK) {
        printf("Unable to establish ssl session.\n");
        goto leaving;
    }

    if (X509_V_OK != SSL_get_verify_result(pSSLHandle)) {
        printf("Server Certificate Verification failed");
        nRet = A71CH_TLS_CLIENT_SSL_CONNECT_ERROR;
        goto leaving;
    }

    // Ensure a valid certificate was returned, otherwise no certificate exchange happened!
    if (NULL == (certRcv = SSL_get_peer_certificate(pSSLHandle)) ) {
        printf(" No certificate exchange happened");
        nRet = A71CH_TLS_CLIENT_SSL_CONNECT_ERROR;
        goto leaving;
    }
    else {
        X509_NAME *subj;
        char szData[256];

        if ((subj = X509_get_subject_name(certRcv)) &&
                (X509_NAME_get_text_by_NID(subj, NID_commonName, szData, 256) > 0)) {
            printf("Peer Certificate CN: %s\n", szData);
        }
        X509_free(certRcv);
    }

    nRet = sslWriteWrapper(pSSLHandle, sockfd, (unsigned char *)clientMessage, &clientMessageLen, 2000);
    if (nRet != A71CH_TLS_CLIENT_OK) {
        printf("Unable to send message to server: nRet = %d.\n", nRet);
        goto leaving;
    }

    nRet = SSL_shutdown(pSSLHandle);
    if (nRet == 0) {
        sleep(1);
        printf("SSL_shutdown: Repeat shutdown.\n");
        nRet = SSL_shutdown(pSSLHandle);
    }

    if (nRet == 1) {
        printf("SSL_shutdown: successful.\n");
        nRet = A71CH_TLS_CLIENT_OK;
    }
    else if (nRet == 0) {
        printf("SSL_shutdown: not yet finished (second attempt).\n");
    }
    else {
        int errorCode;
        errorCode = SSL_get_error(pSSLHandle, nRet);
        printf("SSL_shutdown: error on shutdown (nRet=%d, SSL_error_code=%d)\n",
            nRet, errorCode);
        // if (errorCode == SSL_ERROR_ZERO_RETURN )
    }

    nRet = SSL_clear(pSSLHandle);
    if (nRet == 0) {
        int errorCode;
        printf("SSL_clear failed.\n");
        errorCode = SSL_get_error(pSSLHandle, nRet);
        printf("errorCode: %d\n", errorCode);
    }
    else if (nRet == 1) {
        printf("SSL_clear successful.\n");
        nRet = A71CH_TLS_CLIENT_OK;
    }

    printf("ENGINE_unregister_ECDH(e).\n");
    ENGINE_unregister_ECDH(e);

    // To demonstrate one can alternate OpenSSL engine access and direct access
    // to the A71CH, a direct call to a Data Object stored in GP storage.
    nRet = wrapConnectToA71CH();
    if (nRet != A71CH_TLS_CLIENT_OK) {
        printf("Failed to connect to A71CH.\n");
        goto leaving;
    }

    dataObjectLen = MAX_DATA_OBJECT_SIZE;
    objectIndex = 0;
    dataOffset = 0;
    dataLen = 11;
    nRet = a71GetDataObject(objectIndex, dataOffset, dataLen, dataObject, &dataObjectLen);
    if (nRet != A71CH_TLS_CLIENT_OK) {
        printf("Failed to retrieve data from GP storage.\n");
        goto leaving;
    }
    // The data object at offset 0 is provisioned with a string (so we can print it out)
    printf("Value of data object at index %d (offset=0x%04X; len=%d): %s\n", objectIndex,
        dataOffset, dataLen, dataObject);

    dataOffset = 0x3f8;
    dataLen = 8;
    nRet = a71GetDataObject(objectIndex, dataOffset, dataLen, dataObject, &dataObjectLen);
    if (nRet != A71CH_TLS_CLIENT_OK) {
        printf("Failed to retrieve data from GP storage.\n");
        goto leaving;
    }
    // The data object at offset 0x3f8 is provisioned with a string (so we can print it out)
    // must still add string termination
    dataObject[dataLen] = 0;
    printf("Value of data object at index %d (offset=0x%04X; len=%d): %s\n", objectIndex,
        dataOffset, dataLen, dataObject);

    SM_Close(SMCOM_CLOSE_MODE_STD);

    printf("\n\t servername:port = %s:%d\n", pDestinationURL, nDestinationPort);
    printf("\t rootCA: %s\n", fileRootCA);
    printf("\t Retrieved client certificate from A71CH (stored at index 0)\n");
    printf("\t clientKey: %s\n\n", fileClientKey);

leaving:
    printf("\n******** TLS Client Example (Credentials in A71CH) = %s ********\n",
        (nRet == A71CH_TLS_CLIENT_OK) ? "Pass" : "Fail");
    nStatus = (nRet == A71CH_TLS_CLIENT_OK) ? OK_STATUS : FAILURE_STATUS;

    return nStatus;
}

/* Print a hex array utility */
#if 1
static void printDataAsHex(U8* data, U16 dataLen)
{
    int i;
    for (i=0; i<(int)dataLen; i++)
    {
        if ( (i%16 == 0) && (i != 0) ) { printf("\n"); }
        printf("%02x:", data[i]);
    }
    printf("\n");
}
#endif
