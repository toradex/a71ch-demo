/**
 * @file tst_sm_time.c
 * @author NXP Semiconductors
 * @version 1.0
 * @par LICENSE
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
 * (APDU) Execution time measurement utility library
 * @par HISTORY
 * 1.0   03-apr-2015 : Initial version
 *
 *****************************************************************************/

/*******************************************************************
* standard include files
*******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*******************************************************************
* project specific include files
*******************************************************************/
#include "tst_sm_util.h"
#include "tst_sm_time.h"

/*******************************************************************
* global variables and struct definitions
*******************************************************************/

/*
 * API
 */

#ifdef _WIN32
#define CLOCK_MONOTONIC 0

int clock_gettime(int X, struct timeval *tv)
{
    ULONGLONG msSinceBoot = GetTickCount64();

    tv->tv_sec = msSinceBoot / 1000;
    tv->tv_usec = (msSinceBoot % 1000) * 1000;
    return (0);
}
#endif

/**
 * Initiate measurement of execution time
 *
 * @param[in,out] mPair In: Pointer to an allocated axTimeMeasurement_t structure
 */
void initMeasurement(axTimeMeasurement_t *mPair)
{
    mPair->tStart.tv_sec = 0;
#ifdef _WIN32
    mPair->tStart.tv_usec = 0;
#else
    mPair->tStart.tv_nsec = 0;
#endif
    mPair->tEnd.tv_sec = 0;
#ifdef _WIN32
    mPair->tEnd.tv_usec = 0;
#else
    mPair->tEnd.tv_nsec = 0;
#endif

    clock_gettime(CLOCK_MONOTONIC, &(mPair->tStart));
}

/**
 * Conclude measurement of execution time. Matches a call to ::initMeasurement
 *
 * @param[in,out] mPair In: Pointer to an allocated axTimeMeasurement_t structure
 */
void concludeMeasurement(axTimeMeasurement_t *mPair)
{
    clock_gettime(CLOCK_MONOTONIC, &(mPair->tEnd));
}

/**
 * Calculates (and returns) execution time in ms (milli seconds).
 * \pre Call to ::initMeasurement
 * \pre Call to ::concludeMeasurement
 *
 * @param[in,out] mPair In: Pointer to an allocated axTimeMeasurement_t structure
 * @returns Execution time in ms (milli seconds)
 */
long getMeasurement(axTimeMeasurement_t *mPair)
{
    long startMillis;
    long endMillis;
    long deltaMillis;
    // printf("Start: Sec: 0x%08x - nSec: 0x%08x\n", mPair->tStart.tv_sec, mPair->tStart.tv_nsec);
    // printf("End  : Sec: 0x%08x - nSec: 0x%08x\n", mPair->tEnd.tv_sec, mPair->tEnd.tv_nsec);
#ifdef _WIN32
    startMillis = (mPair->tStart.tv_sec * 1000) + (mPair->tStart.tv_usec / 1000);
    endMillis = (mPair->tEnd.tv_sec * 1000) + (mPair->tEnd.tv_usec / 1000);
#else
    startMillis = (mPair->tStart.tv_sec * 1000) + (mPair->tStart.tv_nsec / 1000000);
    endMillis = (mPair->tEnd.tv_sec * 1000) + (mPair->tEnd.tv_nsec / 1000000);
#endif
    deltaMillis = endMillis - startMillis;

    // printf("Delta: %d ms\n", deltaMillis);
    return deltaMillis;
}

/**
 * Create a report fragment based on measurements contained in \p msArray
 *
 * @param[in] fHandle Valid file handle to contain report fragment
 * @param[in] szMessage Label to use in report fragment
 * @param[in] msArray Array containing (execution time) measurements (in ms)
 * @param[in] nMeasurement Amount of measurements contained in \p msArray
 * @param[in] reportMode Defines style of report (construct bitpattern with e.g. ::AX_MEASURE_REPORT_VERBOSE or ::AX_MEASURE_ECHO_STDOUT)
 */
void axSummarizeMeasurement(FILE *fHandle, char *szMessage, long *msArray, int nMeasurement, int reportMode)
{
    int i;
    long averaged = 0;
    long minValue = 0;
    long maxValue = 0;
    int fEchoStdout = 0;
    int fReportVerbose = 0;
    FILE *fHandleArray[2];
    int nHandle = 2;
    int nOut;

    fEchoStdout = ((reportMode & AX_MEASURE_ECHO_MASK) == AX_MEASURE_ECHO_STDOUT);
    fReportVerbose = ((reportMode & AX_MEASURE_REPORT_MASK) == AX_MEASURE_REPORT_VERBOSE);

    if (fEchoStdout)
    {
        fHandleArray[0] = fHandle;
        fHandleArray[1] = stdout;
        nHandle = 2;
    }
    else
    {
        fHandleArray[0] = fHandle;
        nHandle = 1;
    }

    if (nMeasurement > 0)
    {
        minValue = msArray[0];
        maxValue = msArray[0];
    }
    else
    {
        for (nOut=0; nOut<nHandle; nOut++)
        {
            fprintf(fHandleArray[nOut], "%s: No valid amount of measurements (%d)\n", szMessage, nMeasurement);
        }
        return;
    }

    for (i=0; i<nMeasurement; i++)
    {
        if (fReportVerbose)
        {
            for (nOut=0; nOut<nHandle; nOut++)
            {
                fprintf(fHandleArray[nOut], "%s: %d ms\n", szMessage, msArray[i]);
            }
        }
        averaged += msArray[i];
        minValue = (msArray[i] < minValue) ? msArray[i] : minValue;
        maxValue = (msArray[i] > maxValue) ? msArray[i] : maxValue;
    }
    averaged /= nMeasurement;

    for (nOut=0; nOut<nHandle; nOut++)
    {
        fprintf(fHandleArray[nOut], "Exec Time: %s:\n\tAverage (%d measurements): %d ms\n", szMessage, nMeasurement, averaged);
        fprintf(fHandleArray[nOut], "\tMinimum: %d ms\n", minValue);
        fprintf(fHandleArray[nOut], "\tMaximum: %d ms\n", maxValue);
    }
}

#ifndef TGT_A71CH
/**
 * @param szMessage
 * @param measured
 * @param lowerBound    measured must be higher than lowerBound in case lowerBound is different from 0
 * @param higherBound   measured must be lower than higherBound in case higherBound is different from 0
 * @param severity
 * @return
 */
int evalMeasurement(char *szMessage, long measured, long lowerBound, long higherBound, axExecTimeEval_t severity) {
    int status = 1;
    printf("%s: %d ms\n", szMessage, measured);
    switch (severity) {
        case AX_TIME_EVAL_IGNORE:
            break;
        case AX_TIME_EVAL_WARNING:
        case AX_TIME_EVAL_FATAL:
            if ( (lowerBound != 0) && (measured < lowerBound) ) {
                printf("*** Execution speed faster than specified: %d < %d\n", measured, lowerBound);
                status = (severity == AX_TIME_EVAL_FATAL ? 0 : 1);
            }
            if ( (higherBound != 0)  && (measured > higherBound)) {
                printf("*** Execution speed slower than specified: %d > %d\n", measured, higherBound);
                status = (severity == AX_TIME_EVAL_FATAL ? 0 : 1);
            }
            break;
        default:
            printf("Severity level not defined.\n");
            status = 0;
    }
    return status;
}
#endif //TGT_A71CH
