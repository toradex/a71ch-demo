/**
 * @file axWebUtil.c
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
 * Implementation of Web utility functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// project specific include files
#include "sm_types.h"
#include "axWebUtil.h"

// #define DBG_AX_AWS_UTIL

#ifdef DBG_AX_AWS_UTIL
#define DBGPRINTF(...) printf (__VA_ARGS__)
#else
#define DBGPRINTF(...)
#endif


/**
* Pass a json object and extract the value corresponding to the name.
* This function has severe limitations:
* - Only strings can be extracted
* - No additional checking of format (also non-valid json can be parsed successfully.)
*
* @param[in] szBuf Buffer containing json object
* @param[in] szName Search for a value matching this name
* @param[in,out] szValue IN: buffer to contain value; OUT: In case name matches the corresponding value
* @param[in] bufSize buffer size of szValue
*
* @retval ::SW_OK Upon successful execution
* @retval ::ERR_BUF_TOO_SMALL           Buffer provided is too small to contain szValue
* @retval ::ERR_PATTERN_COMPARE_FAILED  Name not found
*/
int axWebUtilJsonSimpleExtract(char *szBuf, char *szName, char *szValue, U16 bufSize)
{
    char *pChar = NULL;
    char szNamePattern[AX_WEB_MAX_NAME_PATTERN];
    char *pLeading = NULL;
    char *pTrailing = NULL;

//  {"status":"0000","message":"Success","registrationCode":"6b6eb35e359f453ee54421b4b844367d4c17c99fbff791119abda5ad90a93e7b"}
    if (strlen(szName) > (AX_WEB_MAX_NAME_PATTERN-4))
    {
        return ERR_INTERNAL_BUF_TOO_SMALL;
    }

    sprintf(szNamePattern, "\"%s\":", szName);

    pChar = strstr(szBuf, szNamePattern);
    if (pChar == NULL)
    {
        return ERR_PATTERN_COMPARE_FAILED;
    }

    // Jump to the colon (':') just found
    pChar += (strlen(szNamePattern) - 1);

    // Now determine the position of the leading " (containing the value)
    pLeading = strchr(pChar, '"');
    if (pLeading == NULL)
    {
        return ERR_NO_MATCHING_VALUE;
    }

    if (strlen(pLeading) > 0) {
        pLeading++;
    }
    else {
        // Remaining string to short
        return ERR_NO_MATCHING_VALUE;
    }

    // Now determine the position of the closing " (containing the value)
    pTrailing = strchr(pLeading, '"');
    if (pTrailing == NULL)
    {
        return ERR_NO_MATCHING_VALUE;
    }
    pTrailing--;

    if (pTrailing == pLeading)
    {
        *szValue = '\0';
    }
    else if ( (pTrailing - pLeading + 1 + 1) > bufSize)
    {
        return ERR_BUF_TOO_SMALL;
    }
    else
    {
        int i;
        for (i=0; i<(pTrailing - pLeading + 1); i++)
        {
            szValue[i] = pLeading[i];
        }
        szValue[i] = '\0';
    }
    return SW_OK;
}

/**
* Read a certificate (stored in PEM format) from the file handle provided.
* Special handling of newline. Replace one newline character
* by two characters ('\n' -> '\''n')
*
* \pre Filehandle points to PEM file opened for read
* \post cert contains zero terminated newline converted certificate
*
* @param[in,out] cert IN: Buffer to contain certificate; OUT: contains (newline patched) certificate
* @param[in,out] certSize IN: Buffersize; OUT: Actual amount of byte retrieved (excludes 0 byte terminating equivalent string)
* @param[in] fHandle file handle pointing to certificate opened for reading
*
* @retval ::SW_OK Upon successful execution
*/
int axWebUtilReadConvertCert(U8 *cert, U16 *certSize, FILE *fHandle)
{
    int nChar;
    int nCertSize = 0;
    int nCertSizeMax = *certSize;

    while ( (nChar = getc(fHandle)) != EOF )
    {
        if (nChar == '\n')
        {
            // Special handling of newline. Replace one newline character
            // by two characters ('\n' -> '\''n')
            if (nCertSize > nCertSizeMax-3)
            {
                printf("Provided buffer too small.\n");
                *certSize = 0;
                return -3;
            }
            cert[nCertSize++] = (U8)'\\';
            cert[nCertSize++] = (U8)'n';
        }
        else
        {
            if (nCertSize > nCertSizeMax-2)
            {
                printf("Provided buffer too small.\n");
                *certSize = 0;
                return -3;
            }
            cert[nCertSize++] = (U8)nChar;
        }
    }
    cert[nCertSize] = 0;
    *certSize = nCertSize; // Do not include closing '\0' in the byte count

    return SW_OK;
}
