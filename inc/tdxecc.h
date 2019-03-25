/*
 * tdxecc.h
 *
 *  Created on: 19.03.2019
 *      Author: Diego Bienz
 */

#ifndef TDXECC_H_
#define TDXECC_H_

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* a71ch specific includes */
/* required for working with the A71CH secure module */
#include "../a71ch/a71ch_api.h"
#include "../a71ch/api/inc/ax_api.h"
#include "../a71ch/libCommon/infra/sm_types.h"

/* TDXECC tool specific includes */
#define VERSION "[version 1.0]"
#define TOOL_NAME "tdxecc"
#define HW_NAME "Colibri iMX6ULL"
#define	DEV_NAME "/dev/i2c-0"   //This is also defined in libA71CH_i2c.so in the file platform/imx/i2c_a7.c with "static char devName[]"

#define TDXECC_USED_KEYPAIR A71CH_KEY_PAIR_0
#define MAX_HASH_SIZE 128
#define MAX_SIGNATURE_SIZE 128
#define MAX_KEY_SIZE 260
#define TDXECC_OK 0
#define TDXECC_FAIL 1

int tdxecc_verify_signature(const U8* eccPubKey, const U16 eccPubKeyLen, const U8* signedHash,	const U16 signedHashLen, const U8* signature, const U16 signatureLen);
int tdxecc_sign_hash(const U8* hash, const U16 hashLen, U8* signature, U16* signatureLen);
int tdxecc_overwrite_keys(const U8* eccPubKey, const U16 eccPubKeyLen, const U8* eccPrivKey, const U16 eccPrivKeyLen);
void tdxecc_print_info(void);
void tdxecc_print_help(void);
int main (int argc, char * argv[]);

#endif /* TDXECC_H_ */
