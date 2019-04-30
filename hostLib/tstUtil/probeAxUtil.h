/**
 * @file probeAxUtil.h
 * @author NXP Semiconductors
 * @version 1.0
 * @section LICENSE
 * ----------------------------------------------------------------------------
 * Copyright 2016 NXP
 *
 * This software is owned or controlled by NXP and may only be used
 * strictly in accordance with the applicable license terms.  By expressly
 * accepting such terms or by downloading, installing, activating and/or
 * otherwise using the software, you are agreeing that you have read, and
 * that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you
 * may not retain, install, activate or otherwise use the software.
 * ----------------------------------------------------------------------------
 * @section DESCRIPTION
 * API of probe utility functions specific to JCOP2.4.2
 * ----------------------------------------------------------------------------
 * @section HISTORY
 * 1.0   11-may-2016 : Initial version
 *
 *****************************************************************************/
#ifndef _PROBE_AX_UTIL_H_
#define _PROBE_AX_UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sm_types.h"
#include "sm_printf.h"
#include "ax_api.h"

// This module implements generic probe functions
U16 probeAxIdentifyFetchPrint();
U16 probeAxSelectCardmanager();
U16 probeAxGetCplcDataFetchPrint();

#ifdef __cplusplus
}
#endif
#endif // _PROBE_AX_UTIL_H_
