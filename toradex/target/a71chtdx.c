/****************************************************************************
**
** Copyright (C) 2019 Toradex AG
** Contact: https://www.toradex.com/locations
**
** This file is part of the Toradex of the A71CH workshop demo.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Toradex Ag nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
** Precondition:
**  * - A71CH has been provisioned with appropriate credentials
**  * - Server side has been setup and has been provisioned with appropriate credentials
**
****************************************************************************/

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
#include <ifaddrs.h>
#include <fcntl.h>
#include <openssl/ssl.h>
#include <openssl/engine.h>
#include <arpa/inet.h>

#include "sm_types.h"
#include "a71ch_api.h"
#include "ax_api.h"
#include "HLSEAPI.h"

#define _GNU_SOURCE
#define OK_STATUS                                   0
#define FAILURE_STATUS                              1

#define A71CHTDX_STATUS_OK                          0

#define A71CHTDX_TCP_SETUP_ERROR                    100
#define A71CHTDX_TCP_CONNECT_ERROR                  101

#define A71CHTDX_SSL_DEFAULT_TIMEOUT                5000
#define A71CHTDX_SSL_MAX_MESSAGE_SIZE               1024
#define A71CHTDX_SSL_ROOT_CA                        "eccKeys/tls_rootca.cer"
#define A71CHTDX_SSL_CLIENT_KEY                     "eccKeys/tls_client_key.pem"
#define A71CHTDX_SSL_CONNECTED                      1
#define A71CHTDX_SSL_SELECT_TIMEOUT                 0
#define A71CHTDX_SSL_SELECT_ERROR                   -1
#define A71CHTDX_SSL_LIB_INIT_ERROR                 200
#define A71CHTDX_SSL_INIT_ERROR                     201
#define A71CHTDX_SSL_CERT_ERROR                     202
#define A71CHTDX_SSL_KEY_ERROR                      203
#define A71CHTDX_SSL_CONNECT_ERROR                  204
#define A71CHTDX_SSL_CONNECT_TIMEOUT_ERROR          205
#define A71CHTDX_SSL_WRITE_TIMEOUT_ERROR            206
#define A71CHTDX_SSL_WRITE_ERROR                    207
#define A71CHTDX_SSL_READ_ERROR                     208
#define A71CHTDX_SSL_READ_TIMEOUT_ERROR             209

#define A71CHTDX_A71CH_USED_KEYPAIR                 A71CH_KEY_PAIR_0
#define A71CHTDX_A71CH_MAX_HASH_SIZE                128
#define A71CHTDX_A71CH_MAX_SIGNATURE_SIZE           128
#define A71CHTDX_A71CH_MAX_KEY_SIZE                 260
#define A71CHTDX_A71CH_MAX_DER_CERT_SIZE            1024
#define A71CHTDX_A71CH_MAX_CERT_HANDLE              255
#define A71CHTDX_A71CH_GP_ACCESS                    300
#define A71CHTDX_A71CH_GP_NO_CERT                   301
#define A71CHTDX_A71CH_LOADING_ENGINE_ERR           302
#define A71CHTDX_A71CH_ECC_ERROR                    303
#define A71CHTDX_A71CH_ECC_OK                       304
#define A71CHTDX_A71CH_SHA256_ERROR                 305
#define A71CHTDX_A71CH_SHA256_OK                    306

#define A71CHTDX_SHA_MAX_LENGTH                     128
#define A71CHTDX_FILE_WRITE_ERROR                   601
#define A71CHTDX_FILE_WRITE_OK                      602
#define A71CHTDX_DOWNLOAD_INTEGRITY_NOT_PROVEN      701
#define A71CHTDX_DOWNLOAD_INTEGRITY_PROVEN          702


/* Different levels of prints defined */
#define DEBUG                                       0
#define PRINT_ERROR(args...) \
        do { fprintf(stderr, "\e[1;31m"); fprintf(stderr, "ERROR: "); fprintf(stderr, ##args); fprintf(stderr, "\e[0m"); } while (0)

#define PRINT_DEBUG(args...) \
        do { if (DEBUG) {fprintf(stderr, "\e[01;33m"); fprintf(stderr, "DEBUG: "); fprintf(stderr, ##args); fprintf(stderr, "\e[0m"); }} while (0)

#define PRINT_INFO(args...) \
        do { fprintf(stderr, "\e[1;36m"); fprintf(stderr, "INFO: "); fprintf(stderr, ##args); fprintf(stderr, "\e[0m"); } while (0)

/* Wrapper function for the connection process to the A71CH. */
int a71chtdx_connect_wrapper()
{
    U16 connectStatus = 0;
    U8 Atr[64];
    U16 AtrLen = sizeof(Atr);
    SmCommState_t commState;

    connectStatus = SM_Connect(&commState, Atr, &AtrLen);

    if ( (connectStatus == ERR_CONNECT_LINK_FAILED) || (connectStatus == ERR_CONNECT_SELECT_FAILED) )
    {
        PRINT_ERROR("SM_Connect failed with status 0x%04X\n", connectStatus);
        return 2;
    }
    else if ( connectStatus == SMCOM_COM_FAILED )
    {
        PRINT_ERROR("SM_Connect failed with status 0x%04X (Could not open communication channel)\n", connectStatus);
        return 4;
    }
    else if ( connectStatus == SMCOM_PROTOCOL_FAILED)
    {
        PRINT_ERROR("SM_Connect failed with status 0x%04X (Could not establish communication protocol)\n", connectStatus);
        return 5;
    }
    else if ( connectStatus == ERR_NO_VALID_IP_PORT_PATTERN )
    {
        PRINT_ERROR("Pass the IP address and port number as arguments, e.g. \"127.0.0.1:8050\"!\n");
        return 3;
    }
    else
    {
        int i=0;
        PRINT_DEBUG("SCI2C_"); // To highlight the ATR format for SCI2C deviates from ISO7816-3
        if (AtrLen > 0)
        {
            PRINT_DEBUG("ATR=0x");
            for (i=0; i<AtrLen; i++) { PRINT_DEBUG("%02X.", Atr[i]); }
            PRINT_DEBUG("\n");
        }
        PRINT_DEBUG("HostLib Version  : 0x%04X\n", commState.hostLibVersion);
        if (connectStatus != SW_OK)
        {
            PRINT_ERROR("Select failed. SW = 0x%04X\n", connectStatus);
            return 5;
        }
        PRINT_DEBUG("Applet Version   : 0x%04X\n", commState.appletVersion);
        PRINT_DEBUG("SecureBox Version: 0x%04X\n", commState.sbVersion);
    }
    PRINT_DEBUG("========================\n");
    return A71CHTDX_STATUS_OK;
}

/* Method to sign a given hash (in a hash array) with the A71CH */
int a71chtdx_sign_hash(const U8* hash, const U16 hashLen, U8* signature, U16* signatureLen){
	U16 err = 0;
    int res = 0;

	*signatureLen = A71CHTDX_A71CH_MAX_SIGNATURE_SIZE;

    /* Connect to A71CH */
    res = a71chtdx_connect_wrapper();
    if (res != A71CHTDX_STATUS_OK) {
        PRINT_ERROR("Failed to connect to A71CH.\n");
        return -1;
    }

	err = A71_EccSign(A71CHTDX_A71CH_USED_KEYPAIR, hash, hashLen, signature,	signatureLen);
	if (err == SW_OK) {
		return A71CHTDX_A71CH_ECC_OK;
	} else {
        PRINT_ERROR("Unable to sign the hash. Error-Code: %i\n", err);
		return A71CHTDX_A71CH_ECC_ERROR;
	}
    SM_Close(SMCOM_CLOSE_MODE_STD);
}

/* Method to get the ECC public key from the A71CH Secure Element */
int a71chtdx_get_ecc_public_key(U8* publicKey, U16* publicKeyLen){
    U16 err = 0;
    int res = 0;

	*publicKeyLen = A71CHTDX_A71CH_MAX_KEY_SIZE;

    /* Connect to A71CH */
    res = a71chtdx_connect_wrapper();
    if (res != A71CHTDX_STATUS_OK) {
        PRINT_ERROR("Failed to connect to A71CH.\n");
        return -1;
    }

	err = A71_GetPublicKeyEccKeyPair(A71CHTDX_A71CH_USED_KEYPAIR, publicKey, publicKeyLen);
	if (err == SW_OK) {
		return A71CHTDX_A71CH_ECC_OK;
	} else {
		return A71CHTDX_A71CH_ECC_ERROR;
	}
    SM_Close(SMCOM_CLOSE_MODE_STD);
}

/* Calculate the SHA256 from given data on the A71CH Secure Element */
int a71chtdx_calulate_hash(U8* data, U16 dataLen, U8* sha, U16* shaLen){
    U16 err = 0;
    int res = 0;

    *shaLen = A71CHTDX_SHA_MAX_LENGTH;

    /* Connect to A71CH */
    res = a71chtdx_connect_wrapper();
    if (res != A71CHTDX_STATUS_OK) {
        PRINT_ERROR("Failed to connect to A71CH.\n");
        return -1;
    }

	err = A71_GetSha256(data, dataLen, sha, shaLen);
	if (err == SW_OK) {
		return A71CHTDX_A71CH_SHA256_OK;
	} else {
        PRINT_ERROR("Error: %i\n", err);
		return A71CHTDX_A71CH_SHA256_ERROR;
	}
    SM_Close(SMCOM_CLOSE_MODE_STD);
}

/* used for OpenSSL */
int static a71chtdx_cert_verify_cb(int preverifyOk, X509_STORE_CTX *pX509CTX)
{
    char szData[256];
    X509 *cert = X509_STORE_CTX_get_current_cert(pX509CTX);
    int depth = X509_STORE_CTX_get_error_depth(pX509CTX);

    if (!preverifyOk) {
        int err = X509_STORE_CTX_get_error(pX509CTX);
        PRINT_ERROR("Error with certificate at depth: %i\n", depth);
        X509_NAME_oneline(X509_get_issuer_name(cert), szData, 256);
        PRINT_ERROR(" issuer = %s\n", szData);
        X509_NAME_oneline(X509_get_subject_name(cert), szData, 256);
        PRINT_ERROR(" subject = %s\n", szData);
        PRINT_ERROR(" err %i:%s\n", err, X509_verify_cert_error_string(err));
    }
    else {
        PRINT_DEBUG("Certificate at depth: %i\n", depth);
        X509_NAME_oneline(X509_get_issuer_name(cert), szData, 256);
        PRINT_DEBUG(" issuer = %s\n", szData);
        X509_NAME_oneline(X509_get_subject_name(cert), szData, 256);
        PRINT_DEBUG(" subject = %s\n", szData);
    }
    return preverifyOk;
}

/* Wrapper function to write to specific file. */
int static a71chtdx_file_write_wrapper(char* filename, char* fileContent, int contentBufferSize){
    FILE *fp;
    fp = fopen(filename,"wb");

    if(fp == NULL)
    {
        /* File not created hence exit */
        PRINT_ERROR("Unable to create file with name \"%s\".\n", filename);
        return A71CHTDX_FILE_WRITE_ERROR;
    }
    else{
        fwrite(fileContent, contentBufferSize, 1, fp);
        fclose(fp);
        return A71CHTDX_FILE_WRITE_OK;
    }
}

/* Converts a string of hex values into an array of hex values. */
void static a71chtdx_convert_char_to_hex_array(const char* input, U8* output, U16* outputSize) {
	unsigned int uChar = 0;
    *outputSize = 0;
    int i = 0;

	for (int j = 0;i < strlen(input)-1; i = i + 2, j++) {
		sscanf(&input[i], "%02x", &uChar);
		*outputSize = *outputSize + 1;
		output[j] = uChar;
	}
}

/* Converts an array of hex values into a string of hex values. */
void static a71chtdx_convert_hex_to_char_array(const U8* input, const U16 inputSize, char* output) {
	for (int i = 0; i < inputSize; i++) {
		sprintf(output+(i*2), "%02x", input[i]);
	}
    output[inputSize*2] = '\0';
}

/* Method to extract hash out of specific server response */
void static a71chtdx_extract_hash(char *originalMessage, U8 *hash, U16 *hashLen){
    char hashString[A71CHTDX_A71CH_MAX_HASH_SIZE];
    char* startOfHash;
    char* endOfHash;

    startOfHash = strchr(originalMessage, '=')+1;
    endOfHash = strchr(startOfHash, '\n');
    if (endOfHash) *endOfHash = '\0';
    *hashLen = strlen(startOfHash);
    memcpy(hashString, startOfHash, ((*hashLen)+1) * sizeof(char));
    a71chtdx_convert_char_to_hex_array(hashString, hash, hashLen);
}

/* Method to extract different values out of specific server response */
void static a71chtdx_extract_file_metadata(char *originalMessage, unsigned int *filesize, U8 *hash, U16 *hashLen, char *filename, char **startOfContent){
    char hashString[A71CHTDX_A71CH_MAX_HASH_SIZE];
    char filesizeString[64];
    char* startOfSection;
    char* endOfSection;

    // Extraction of filesize
    PRINT_DEBUG("Extracting file size...\n");
    startOfSection = strchr(originalMessage, '=')+1;
    endOfSection = strchr(startOfSection, '\n');
    if (endOfSection) *endOfSection = '\0';
    memcpy(filesizeString, startOfSection, sizeof(char) * strlen(startOfSection));
    PRINT_DEBUG("File size in string: %s\n", filesizeString);
    *filesize = atoi(filesizeString);
    PRINT_DEBUG("Filesize: %i\n", *filesize);

    // Extraction of filename
    PRINT_DEBUG("Extracting file name...\n");
    endOfSection ++;
    startOfSection = strchr(endOfSection, '=')+1;
    endOfSection = strchr(startOfSection, '\n');
    if (endOfSection) *endOfSection = '\0';
    memcpy(filename, startOfSection, sizeof(char) * strlen(startOfSection));
    PRINT_DEBUG("Filename: %s\n", filename);

    // Extraction of hash
    PRINT_DEBUG("Extracting hash of file...\n");
    endOfSection ++;
    startOfSection = strchr(endOfSection, '=')+1;
    endOfSection = strchr(startOfSection, '\n');
    *endOfSection = '\0';
    *hashLen = strlen(startOfSection);
    memcpy(hashString, startOfSection, (*hashLen + 1) * sizeof(char));
    PRINT_DEBUG("Extracted hash with len %i: %s\n", *hashLen, hashString);
    a71chtdx_convert_char_to_hex_array(hashString, hash, hashLen);

    // Extraction of file
    endOfSection ++;
    *startOfContent = endOfSection;
}

/* Wrapper function to connect to the tcp socket */
static int a71chtdx_tcp_connect_wrapper(int socket_fd, char *pURLString, int port)
{
    int nRet = A71CHTDX_TCP_CONNECT_ERROR;
    int connect_status = -1;
    struct hostent *host;
    struct sockaddr_in dest_addr;

    if( (host = gethostbyname(pURLString)) == NULL){
        PRINT_ERROR("Error in resolving hostname \"%s\"\n", pURLString);
        return nRet;
    }

    if (NULL != host) {
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(port);
        dest_addr.sin_addr.s_addr = *(long*) (host->h_addr);
        memset(&(dest_addr.sin_zero), '\0', 8);

        connect_status = connect(socket_fd, (struct sockaddr *) &dest_addr,
                sizeof(struct sockaddr));
        if (connect_status != -1) {
            nRet = A71CHTDX_STATUS_OK;
        }
    }
    PRINT_DEBUG("Connect Status: %i\n", connect_status);
    return nRet;
}

/* Wrapper function to set the tcp connection to non-blocking */
static int a71chtdx_set_tcp_non_blocking_wrapper(int server_fd)
{
    int flags, status;
    int nRet = A71CHTDX_STATUS_OK;

    flags = fcntl(server_fd, F_GETFL, 0);
    // set underlying socket to non blocking
    if (flags < 0) {
        nRet = A71CHTDX_TCP_CONNECT_ERROR;
    }

    status = fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);
    if (status < 0) {
        PRINT_ERROR("fcntl - %s", strerror(errno));
        nRet = A71CHTDX_TCP_CONNECT_ERROR;
    }

    return nRet;
}

/* Wrapper function to connect to the ssl socket */
static int a71chtdx_ssl_connect_wrapper(SSL *pSSL, int server_fd, int timeout_ms)
{
    int nRet = A71CHTDX_STATUS_OK;
    int rc = 0;
    fd_set readFds;
    fd_set writeFds;
    struct timeval timeout = { timeout_ms / 1000, (timeout_ms % 1000) * 1000 };
    int errorCode = 0;
    int nSelect = A71CHTDX_SSL_SELECT_TIMEOUT;

    do {
        PRINT_DEBUG("Invoke SSL_connect()...\n");
        rc = SSL_connect(pSSL);

        if (A71CHTDX_SSL_CONNECTED == rc) {
            nRet = A71CHTDX_STATUS_OK;
            break;
        }

        errorCode = SSL_get_error(pSSL, rc);

        if (errorCode == SSL_ERROR_WANT_READ) {
            FD_ZERO(&readFds);
            FD_SET(server_fd, &readFds);
            nSelect = select(server_fd + 1, (void *) &readFds, NULL, NULL, &timeout);
            if (A71CHTDX_SSL_SELECT_TIMEOUT == nSelect) {
                PRINT_ERROR("a71chtdx_ssl_connect_wrapper: time out while waiting for read.\n");
                nRet = A71CHTDX_SSL_CONNECT_TIMEOUT_ERROR;
            }
            else if (A71CHTDX_SSL_SELECT_ERROR == nSelect) {
                PRINT_ERROR("a71chtdx_ssl_connect_wrapper: select error for read %d.\n", nSelect);
                nRet = A71CHTDX_SSL_CONNECT_ERROR;
            }
        }
        else if (errorCode == SSL_ERROR_WANT_WRITE) {
            FD_ZERO(&writeFds);
            FD_SET(server_fd, &writeFds);
            nSelect = select(server_fd + 1, NULL, (void *) &writeFds, NULL, &timeout);
            if (A71CHTDX_SSL_SELECT_TIMEOUT == nSelect) {
                PRINT_ERROR("a71chtdx_ssl_connect_wrapper: time out while waiting for write.\n");
                nRet = A71CHTDX_SSL_CONNECT_TIMEOUT_ERROR;
            }
            else if (A71CHTDX_SSL_SELECT_ERROR == nSelect) {
                PRINT_ERROR("a71chtdx_ssl_connect_wrapper: select error for write %d.\n", nSelect);
                nRet = A71CHTDX_SSL_CONNECT_ERROR;
            }
        }
        else {
            nRet = A71CHTDX_SSL_CONNECT_ERROR;
        }

    } while ( (A71CHTDX_SSL_CONNECT_ERROR != nRet) &&
              (A71CHTDX_SSL_CONNECT_TIMEOUT_ERROR != nRet) );

    return nRet;
}

/* Wrapper function to write to ssl socket */
static int a71chtdx_ssl_write_wrapper(SSL *pSSL, int server_fd, unsigned char *msg, int *totalLen, int timeout_ms)
{
    int nRet = A71CHTDX_STATUS_OK;
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
            PRINT_DEBUG("Wrote %i of %i chars to SSL socket.\n", writtenLen, *totalLen);
        }
        else if (errorCode == SSL_ERROR_WANT_WRITE) {
            PRINT_ERROR("****** error in writing ... \n");
            FD_ZERO(&writeFds);
            FD_SET(server_fd, &writeFds);
            nSelect = select(server_fd + 1, NULL, (void *) &writeFds, NULL, &timeout);
            if (A71CHTDX_SSL_SELECT_TIMEOUT == nSelect) {
                nRet = A71CHTDX_SSL_WRITE_TIMEOUT_ERROR;
            }
            else if (A71CHTDX_SSL_SELECT_ERROR == nSelect) {
                nRet = A71CHTDX_SSL_WRITE_ERROR;
            }
        }
        else {
            nRet = A71CHTDX_SSL_WRITE_ERROR;
        }

    } while ( (A71CHTDX_SSL_WRITE_ERROR != nRet) &&
              (A71CHTDX_SSL_WRITE_TIMEOUT_ERROR != nRet) &&
              (writtenLen < *totalLen) );

    *totalLen = writtenLen;

    return nRet;
}

/* Wrapper function to read from ssl socket */
static int a71chtdx_ssl_read_wrapper(SSL *pSSL, int server_fd, char *buffer, int bufferLen, int *readLen, int timeout_ms)
{
    int nRet = A71CHTDX_STATUS_OK;
    fd_set readFds;
    int errorCode = 0;
    int nSelect;
    int rc = 0;
    struct timeval timeout = { timeout_ms / 1000, (timeout_ms % 1000) * 1000 };

    *readLen = 0;
    do {
        rc = SSL_read(pSSL, &buffer[*readLen], bufferLen-*readLen);   /*This also works if the maximum buffer size for SSL_read (16384 bytes) is exceeded */
        errorCode = SSL_get_error(pSSL, rc);
        if (0 < rc) {
            *readLen += rc;
        }
        else if (errorCode == SSL_ERROR_WANT_READ) {
            FD_ZERO(&readFds);
            FD_SET(server_fd, &readFds);
            nSelect = select(server_fd + 1, (void *) &readFds, NULL, NULL, &timeout);
            if (A71CHTDX_SSL_SELECT_TIMEOUT == nSelect) {
                nRet = A71CHTDX_SSL_READ_TIMEOUT_ERROR;
            }
            else if (A71CHTDX_SSL_SELECT_ERROR == nSelect) {
                nRet = A71CHTDX_SSL_READ_ERROR;
            }
        }
        else {
            nRet = A71CHTDX_SSL_READ_ERROR;
        }

    } while ((A71CHTDX_SSL_READ_ERROR != nRet) &&
            (A71CHTDX_SSL_READ_TIMEOUT_ERROR != nRet) &&
            *readLen < bufferLen);

    if (*readLen > 0){
        nRet = A71CHTDX_STATUS_OK;
    }

    return nRet;
}

/* Method to get my local IP */
static int a71chtdx_get_local_ip(char* localIp){
    int res = 1;
    int n;
    struct ifaddrs *ifaddr, *ifa;
    struct sockaddr_in *sa;
    char* addr;

    res = getifaddrs(&ifaddr);
    if(res != 0){
        PRINT_ERROR("Unable to get local IP\n");
    }
    else{

        /* Walk through linked list, maintaining head pointer so we
              can free list later */

        for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {
            if (ifa->ifa_addr == NULL){
                continue;
            }
            if (ifa->ifa_addr->sa_family==AF_INET) {
                if (strcmp(ifa->ifa_name, "eth0") == 0){
                    sa = (struct sockaddr_in *) ifa->ifa_addr;
                    addr = inet_ntoa(sa->sin_addr);
                    break;
                }
            }
        }
        memcpy(localIp, addr, NI_MAXHOST);
        freeifaddrs(ifaddr);
    }
    return res;
}

/* Enumerates all the Objects that currently exist on the Secure Element and have objectType type. A list of object handles is returned in objectsHandles. */
int a71chtdx_get_client_certificate(int certIndex, U8 *clientCertDer, U16 *clientCertDerLen)
{
    int i;
    int handleWasSet = 0;
    HLSE_RET_CODE hlseRc;
    HLSE_ATTRIBUTE attr;
    HLSE_OBJECT_HANDLE certHandles[A71CHTDX_A71CH_MAX_CERT_HANDLE];
    HLSE_OBJECT_HANDLE readHandle;
    U16 certHandlesNum = sizeof(certHandles) / sizeof(HLSE_OBJECT_HANDLE);

    // Enumerate handles
    certHandlesNum = sizeof(certHandles) / sizeof(HLSE_OBJECT_HANDLE);
    hlseRc = HLSE_EnumerateObjects(HLSE_CERTIFICATE, certHandles, &certHandlesNum);
    if (hlseRc != HLSE_SW_OK) { return A71CHTDX_A71CH_GP_ACCESS; }

    // We are looking for the (certificate) object on certIndex;
    PRINT_DEBUG("getCertificate on index %d\n", certIndex);

    // Find handle
    for (i = 0; i < certHandlesNum; i++) {
        // printf("Looking at index %d: 0x%02X <> 0x%02X\n", i, (certHandles[i] & 0xF), curHandle);
        if (HLSE_GET_OBJECT_INDEX(certHandles[i]) == (U8)certIndex) {
            readHandle = certHandles[i];
            handleWasSet = 1;
            break;
        }
    }
    if (!handleWasSet) { return A71CHTDX_A71CH_GP_NO_CERT; }

    // Read
    attr.type = HLSE_ATTR_OBJECT_VALUE;
    attr.value = clientCertDer;
    attr.valueLen = *clientCertDerLen;
    hlseRc = HLSE_GetObjectAttribute(readHandle, &attr);
    if (hlseRc != HLSE_SW_OK) {
        return A71CHTDX_A71CH_GP_NO_CERT;
    }

    *clientCertDerLen = attr.valueLen;
    PRINT_DEBUG("Size of certificate retrieved=%d\n", *clientCertDerLen);

    return A71CHTDX_STATUS_OK;
}

/* Method with the full process of authentication, verification and download of the file */
int a71chtdx_execute_controlled_download(char* pDestinationURL, int nDestinationPort, char* targetFilename)
{
    /*Status*/
    int integrityStatus = A71CHTDX_DOWNLOAD_INTEGRITY_NOT_PROVEN;
    int nRet = A71CHTDX_STATUS_OK;

    /*SSL properties*/
    ENGINE *e;
    const SSL_METHOD *method;
    SSL_CTX *pSSLContext;
    SSL *pSSLHandle;
    int sockfd;
    X509 *certRcv;
    char fileRootCA[256] = A71CHTDX_SSL_ROOT_CA;
    char fileClientKey[256] = A71CHTDX_SSL_CLIENT_KEY;

    /*Properties for the communication with the server*/
    char localhost[NI_MAXHOST] = "localhost";

    char* writeBuffer;  //Buffer needs to be allocated before using it
    int writeBufferSize;
    int writeBufferStrLen;
    char message_payload[A71CHTDX_SSL_MAX_MESSAGE_SIZE];
    char *message_structure =   "POST / HTTP/1.1\r\n" \
                                "Host:%s\r\n" \
                                "Content-Type: text/plain\r\n" \
                                "Content-Length: %i\r\n" \
                                "Connection: keep-alive\r\n"
                                "\r\n" \
                                "%s";

    char* readBuffer;   //Buffer needs to be allocated before using it
    int readBufferSize;
    int readBytes;

    /*Authorization challenge*/
    U8 challengeHash[A71CHTDX_A71CH_MAX_HASH_SIZE];
    U16 challengeHashLen = 0;
    char challengeHashStr[A71CHTDX_A71CH_MAX_HASH_SIZE];
    U8 challengeSignature[A71CHTDX_A71CH_MAX_SIGNATURE_SIZE];
    U16 challengeSignatureLen = 0;
    char challengeSignatureStr[A71CHTDX_A71CH_MAX_SIGNATURE_SIZE];

    /*File specific*/
    U8 fileHashReceived[A71CHTDX_SHA_MAX_LENGTH];
    U16 fileHashReceivedLen = 0;
    char fileHashReceivedStr[A71CHTDX_SHA_MAX_LENGTH];
    unsigned char fileHashCalculated[SHA256_DIGEST_LENGTH];
    char fileHashCalculatedStr[65];

    /*A71CH specific*/
    int certIndex = A71CHTDX_A71CH_USED_KEYPAIR;
    U8 clientCerDer[A71CHTDX_A71CH_MAX_DER_CERT_SIZE];
    U16 clientCerDerLen = A71CHTDX_A71CH_MAX_DER_CERT_SIZE;

    /* Start */
    PRINT_INFO("******** START OF A71CH-DEMO-DOWNLOAD-UTILITY ********\n");
    a71chtdx_get_local_ip(localhost);

    PRINT_INFO("Setup the secure connection to %s.\n", pDestinationURL);
    /* Connect to A71CH */
    nRet = a71chtdx_connect_wrapper();
    if (nRet != A71CHTDX_STATUS_OK) {
        PRINT_ERROR("Failed to connect to A71CH.\n");
        return -1;
    }

    /* Get the client's certificate */
    nRet = a71chtdx_get_client_certificate(certIndex, clientCerDer, &clientCerDerLen);
    if (nRet != A71CHTDX_STATUS_OK) {
        PRINT_ERROR("Failed to retrieve client certificate.\n");
        goto leaving;
    }


    /* I think connection is not needed anymore */
    SM_Close(SMCOM_CLOSE_MODE_STD);

    /* Load OpenSSL and the engine */
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    if (SSL_library_init() < 0) {
        nRet = A71CHTDX_SSL_LIB_INIT_ERROR;
        goto leaving;
    }
    method = TLSv1_2_method();

    if ((pSSLContext = SSL_CTX_new(method)) == NULL) {
        PRINT_ERROR("SSL_CTX_new failed - Unable to create SSL Context\n");
        nRet = A71CHTDX_SSL_INIT_ERROR;
        goto leaving;
    }

    if (!(e = ENGINE_by_id("e2a71ch")))
    {
        fprintf(stderr, "Error finding OpenSSL Engine by id (id = %s)\n", "e2a71ch");
        nRet = A71CHTDX_A71CH_LOADING_ENGINE_ERR;
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
        nRet = A71CHTDX_TCP_SETUP_ERROR;
        goto leaving;
    }

    if (!SSL_CTX_load_verify_locations(pSSLContext, fileRootCA, NULL)) {
        nRet = A71CHTDX_SSL_CERT_ERROR;
        goto leaving;
    }

    if (!SSL_CTX_use_certificate_ASN1(pSSLContext, clientCerDerLen, clientCerDer)) {
        PRINT_ERROR("Client Certificate Loading error.\n");
        nRet = A71CHTDX_SSL_CERT_ERROR;
        goto leaving;
    }

    if (SSL_CTX_use_PrivateKey_file(pSSLContext, fileClientKey, SSL_FILETYPE_PEM) != 1){
        PRINT_ERROR("Client Private Key Loading error.\n");
        nRet = A71CHTDX_SSL_KEY_ERROR;
        goto leaving;
    }

    SSL_CTX_set_verify(pSSLContext, SSL_VERIFY_PEER, a71chtdx_cert_verify_cb);

    pSSLHandle = SSL_new(pSSLContext);

    /* TCP Connection */
    nRet = a71chtdx_tcp_connect_wrapper(sockfd, pDestinationURL, nDestinationPort);
    if (nRet != A71CHTDX_STATUS_OK) {
        PRINT_ERROR("TCP Connection error.\n");
        goto leaving;
    }

    SSL_set_fd(pSSLHandle, sockfd);

    nRet = a71chtdx_set_tcp_non_blocking_wrapper(sockfd);
    if (nRet != A71CHTDX_STATUS_OK) {
        PRINT_ERROR("Unable to set the socket to Non-Blocking.\n");
        goto leaving;
    }

    nRet = a71chtdx_ssl_connect_wrapper(pSSLHandle, sockfd, 2000);
    if (nRet != A71CHTDX_STATUS_OK) {
        PRINT_ERROR("Unable to establish ssl session.\n");
        goto leaving;
    }

    if (X509_V_OK != SSL_get_verify_result(pSSLHandle)) {
        PRINT_ERROR("Server Certificate Verification failed");
        nRet = A71CHTDX_SSL_CONNECT_ERROR;
        goto leaving;
    }

    // Ensure a valid certificate was returned, otherwise no certificate exchange happened!
    if (NULL == (certRcv = SSL_get_peer_certificate(pSSLHandle)) ) {
        PRINT_ERROR(" No certificate exchange happened");
        nRet = A71CHTDX_SSL_CONNECT_ERROR;
        goto leaving;
    }
    else {
        X509_NAME *subj;
        char szData[256];

        if ((subj = X509_get_subject_name(certRcv)) &&
                (X509_NAME_get_text_by_NID(subj, NID_commonName, szData, 256) > 0)) {
            PRINT_DEBUG("Peer Certificate CN: %s\n", szData);
        }
        X509_free(certRcv);
    }
    /* SSL Connection should now be established! *********************** */
    PRINT_INFO("SSL connection established.\n");

    /* Send the first request to the server. He will answer with the details for the authentication. */
    writeBufferSize = 1024 * sizeof(char);
    writeBuffer = (char*) malloc(writeBufferSize);

    PRINT_INFO("Send request for authorization to server.\n");
    sprintf(message_payload, "command=challenge-request");
    sprintf(writeBuffer, message_structure, localhost, strlen(message_payload), message_payload);
    writeBufferStrLen = strlen(writeBuffer);

    nRet = a71chtdx_ssl_write_wrapper(pSSLHandle, sockfd, (unsigned char *)writeBuffer, &writeBufferStrLen, A71CHTDX_SSL_DEFAULT_TIMEOUT);
    if (nRet != A71CHTDX_STATUS_OK) {
        PRINT_ERROR("Unable to send message to server: nRet = %d.\n", nRet);
        goto leaving;
    }
    free(writeBuffer);

    readBufferSize = 1048576 * sizeof(char); //1M
    readBuffer = (char*) malloc(readBufferSize);
    /* Get the authentication details in the answer of the server. */
    nRet = a71chtdx_ssl_read_wrapper(pSSLHandle, sockfd, readBuffer, readBufferSize, &readBytes, A71CHTDX_SSL_DEFAULT_TIMEOUT);
    if (nRet != A71CHTDX_STATUS_OK) {
        PRINT_ERROR("Unable to read message from server: nRet = %d.\n", nRet);
        goto leaving;
    }
    PRINT_INFO("Got an answer from the server.\n");

    /* Extract the hash from the answer and sign it. */
    a71chtdx_extract_hash(readBuffer, challengeHash, &challengeHashLen);
    a71chtdx_convert_hex_to_char_array(challengeHash, challengeHashLen, challengeHashStr);
    PRINT_INFO("Hash I need to sign: %s\n", challengeHashStr);
    nRet = a71chtdx_sign_hash(challengeHash, challengeHashLen, challengeSignature, &challengeSignatureLen);
    if (nRet != A71CHTDX_A71CH_ECC_OK) {
        PRINT_ERROR("Unable to sign the hash on the A71CH: nRet = %d.\n", nRet);
        goto leaving;
    }
    a71chtdx_convert_hex_to_char_array(challengeSignature, challengeSignatureLen, challengeSignatureStr);
    PRINT_INFO("Resulting Signature: %s\n", challengeSignatureStr);
    free(readBuffer);

    /* Send my signature and the filename of the requested file to the server */
    writeBufferSize = 1024 * sizeof(char);
    writeBuffer = (char*) malloc(writeBufferSize);

    PRINT_INFO("Send authorization details to server.\n");
    sprintf(message_payload, "signature=%s&filename=%s", challengeSignatureStr, targetFilename);
    sprintf(writeBuffer, message_structure, localhost, strlen(message_payload), message_payload);
    PRINT_DEBUG("Full client message I will send: \n%s\n", writeBuffer);
    writeBufferStrLen = strlen(writeBuffer);

    nRet = a71chtdx_ssl_write_wrapper(pSSLHandle, sockfd, (unsigned char *)writeBuffer, &writeBufferStrLen, A71CHTDX_SSL_DEFAULT_TIMEOUT);
    if (nRet != A71CHTDX_STATUS_OK) {
        PRINT_ERROR("Unable to send message to server: nRet = %d.\n", nRet);
        goto leaving;
    }
    free(writeBuffer);

    /* Receive data from server, extract the required information and store the file on the host. */
    readBufferSize = 104857600 * sizeof(char); //100M
    readBuffer = (char*) malloc(readBufferSize);
    unsigned int filesize = 0;
    char* readCursor;

    if (remove(targetFilename) == 0){
        PRINT_DEBUG("Removed existing file %s.\n", targetFilename);
    }

    PRINT_INFO("Waiting for answer from server.\n");
    PRINT_DEBUG("Size of read buffer: %i\n", readBufferSize);
    nRet = a71chtdx_ssl_read_wrapper(pSSLHandle, sockfd, readBuffer, readBufferSize, &readBytes, A71CHTDX_SSL_DEFAULT_TIMEOUT);
    readCursor = &readBuffer[0];
    if (nRet != A71CHTDX_STATUS_OK) {
        PRINT_ERROR("Unable to read message from server: nRet = %d.\n", nRet);
        goto leaving;
    }
    PRINT_DEBUG("Extract file size, file name and hash of file.\n");
    a71chtdx_extract_file_metadata(readBuffer, &filesize, fileHashReceived, &fileHashReceivedLen, targetFilename, &readCursor);
    PRINT_INFO("Received a file with %i bytes.\n", filesize);

    PRINT_INFO("Calculating the SHA256 of the received file.\n");
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, readCursor, filesize);
    SHA256_Final(fileHashCalculated, &sha256);
    a71chtdx_convert_hex_to_char_array(fileHashCalculated, SHA256_DIGEST_LENGTH, fileHashCalculatedStr);
    fileHashCalculatedStr[64] = 0;

    a71chtdx_convert_hex_to_char_array(fileHashReceived, fileHashReceivedLen, fileHashReceivedStr);

    PRINT_INFO("Hash calculated locally: \t\t%s\n", fileHashCalculatedStr);
    PRINT_INFO("Hash received from server: \t%s\n", fileHashReceivedStr);

    if(strcmp(fileHashReceivedStr, fileHashCalculatedStr)==0){
        integrityStatus = A71CHTDX_DOWNLOAD_INTEGRITY_PROVEN;
        PRINT_INFO("Integrity of file is proven.\n");
    }else{
        integrityStatus = A71CHTDX_DOWNLOAD_INTEGRITY_NOT_PROVEN;
        PRINT_ERROR("Integrity of file is NOT proven!\n");
    }

    /* Shutdown of the connection. ************************** */
    nRet = SSL_shutdown(pSSLHandle);

    if (nRet == 1) {
        PRINT_DEBUG("SSL_shutdown: successful.\n");
        nRet = A71CHTDX_STATUS_OK;
    }else{
        PRINT_DEBUG("SSL_shutdown was somehow not successful.\n");
    }

    /* Clear SSL env */
    nRet = SSL_clear(pSSLHandle);
    if (nRet == 0) {
        int errorCode;
        PRINT_ERROR("SSL_clear failed.\n");
        errorCode = SSL_get_error(pSSLHandle, nRet);
        PRINT_ERROR("errorCode: %d\n", errorCode);
    }
    else if (nRet == 1) {
        PRINT_DEBUG("SSL_clear successful.\n");
        nRet = A71CHTDX_STATUS_OK;
    }

    PRINT_DEBUG("ENGINE_unregister_ECDH(e).\n");
    ENGINE_unregister_ECDH(e);

    if(integrityStatus == A71CHTDX_DOWNLOAD_INTEGRITY_PROVEN){
        PRINT_DEBUG("Write data to file.\n");
        nRet = a71chtdx_file_write_wrapper(targetFilename, readCursor, filesize);
        if (nRet == A71CHTDX_FILE_WRITE_OK){
            PRINT_INFO("Wrote content from server to the file \"%s\".\n", targetFilename);
            PRINT_INFO("Secure download successful!\n");
            nRet = A71CHTDX_STATUS_OK;
        }
    }
    else{
        if(strncmp(readCursor, "File not available.", sizeof("File not available."))==0){
            PRINT_ERROR("File \"%s\" is not available on the server.\n", targetFilename);
        }
        nRet = A71CHTDX_DOWNLOAD_INTEGRITY_NOT_PROVEN;
    }
    free(readBuffer);

    leaving:
    PRINT_INFO("******** END OF A71CH-DEMO-DOWNLOAD-UTILITY ********\n");
    return (nRet == A71CHTDX_STATUS_OK) ? OK_STATUS : FAILURE_STATUS;
}

void a71chtdx_print_help(void) {
	printf(
			"\nUsage: a71chtdx [PARAMETER 1] [PARAMETER 2] [PARAMETER 3]\n\n"
					"This software is part of a demonstration for the A71CH Secure Element chip in combination with\n"
                    "the Toradex Colibri iMX6ULL running on Torizon. The software is heavily customized and probably\n"
                    "only working properly in this specific use case of this demo. Anyway, it can certainly be used\n"
                    "to adapt the software for your own application.\n\n"

					"To properly connect to the specific python webserver (part of the demo), you need to define\n"
                    "server address, server port and filename:\n\n"
					"\t -s\tserver name or IP address\n"
                    "\t -p\tserver port\n"
                    "\t -f\tname of the file, which should be downloaded\n\n");
}

int main(int argc, char * argv[]) {
	int c;
    char server[256];
    char filename[256];
    char portStr[16];
    int port;

    if (argc-1 != 6) {
		a71chtdx_print_help();
		return EXIT_SUCCESS;
	}

    while ((c = getopt(argc, &argv[0], "s:f:p:")) != -1) {
        switch (c) {

        case 's': /* hash */
            sprintf(server, "%s", optarg);
            break;

        case 'f':
            sprintf(filename, "%s", optarg);
            break;

        case 'p':
            sprintf(portStr, "%s", optarg);
            port = atoi(portStr);
            break;

        default:
            a71chtdx_print_help();
            return EXIT_FAILURE;
        }
	}

    PRINT_DEBUG("Input: server = %s\n", server);
    PRINT_DEBUG("Input: port = %i\n", port);
    PRINT_DEBUG("Input: filename = %s\n", filename);

    return a71chtdx_execute_controlled_download(server, port, filename);
}
