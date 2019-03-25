/*
 * tdxecc.c
 *
 *  Created on: 19.03.2019
 *      Author: Diego Bienz
 */

#include "../inc/tdxecc.h"

/*Variables required for connecting to A71CH chip */
SmCommState_t commState;
U16 connectStatus;
U8 Atr[64];
U16 AtrLen = sizeof(Atr);

void print_hex_array(const U8* array, U16 lengthOfArray) {
	int i = 0;
	printf("0x");
	for (i = 0; i < lengthOfArray; i++) {
		printf("%02x", array[i]);
	}
}

void read_hex_value(char* input, U8* outputArray, U16* outPutArraySize) {
	int i = 0;
	unsigned int uChar = 0;

	if (input[0] == '0' && input[1] == 'x') {
		i = 2;
	}
	for (int j = 0; i < strlen(input); i = i + 2, j++) {
		sscanf(&input[i], "%02x", &uChar);
		*outPutArraySize = *outPutArraySize + 1;
		outputArray[j] = uChar;
	}
}

int tdxecc_verify_signature(const U8* eccPubKey, const U16 eccPubKeyLen,
		const U8* signedHash, const U16 signedHashLen, const U8* signature,
		const U16 signatureLen) {
	U16 err = 0;
	U8 isOk = 0;

	connectStatus = SM_Connect(&commState, Atr, &AtrLen);
	usleep(1000);

	err = A71_EccVerifyWithKey(eccPubKey, eccPubKeyLen, signedHash,
			signedHashLen, signature, signatureLen, &isOk);
	if (err == SW_OK && isOk == 0x01) {
		return TDXECC_OK;
	} else {
		return TDXECC_FAIL;
	}
}

int tdxecc_sign_hash(const U8* hash, const U16 hashLen, U8* signature,
		U16* signatureLen) {
	U16 err = 0;
	*signatureLen = MAX_SIGNATURE_SIZE;

	connectStatus = SM_Connect(&commState, Atr, &AtrLen);
	usleep(1000);

	err = A71_EccSign(TDXECC_USED_KEYPAIR, hash, hashLen, signature,
			signatureLen);
	if (err == SW_OK) {
		return TDXECC_OK;
	} else {
		return TDXECC_FAIL;
	}
}

int tdxecc_overwrite_keys(const U8* eccPubKey, const U16 eccPubKeyLen,
		const U8* eccPrivKey, const U16 eccPrivKeyLen) {
	U16 volatile err = 0;

	connectStatus = SM_Connect(&commState, Atr, &AtrLen);
	usleep(1000);

	err = A71_SetEccKeyPair(TDXECC_USED_KEYPAIR, eccPubKey, eccPubKeyLen,
			eccPrivKey, eccPrivKeyLen);
	if (err == SW_OK) {
		return TDXECC_OK;
	} else {
		return TDXECC_FAIL;
	}
}

void tdxecc_print_info(void) {
	U16 volatile err = 0;
	U8 uid[A71CH_MODULE_UNIQUE_ID_LEN];
	U16 uidLen = sizeof(uid);

	printf("\nVERSION: %s\n", VERSION);
	printf("Compiled for %s with i2c-device \"%s\"\n", HW_NAME, DEV_NAME);

	connectStatus = SM_Connect(&commState, Atr, &AtrLen);
	usleep(1000);
	err = A71_GetUniqueID(uid, &uidLen);

	if (err == SW_OK) {
		printf("UID of the connected A71CH module is: ");
		print_hex_array(uid, uidLen);
		printf("\n");
	} else {
		printf("Error: There is may no A71CH security module connected. \n\n");
	}
	printf("\n");
}

void tdxecc_print_help(void) {
	printf(
			"\nUsage: tdxecc [ARGUMENT] -[PARAMETER 1] -[PARAMETER 2] ...\n\n"
					"Toradex ECC Utility: Tool to work with the NxP A71CH security module with the Toadex hardware.\n"
					"Please use one of the following switches:\n"
					"\n\tsign\t\tsign a hash with the stored private key\n"
					"\t -h\t\thash\n"
					"\n\tverify\t\tverify a signature with a given hash and a public key\n"
					"\t -s\t\tsignature\n"
					"\t -h\t\thash\n"
					"\t -p\t\tECC public key\n"
					"\n\toverwritekeys\toverwrite they keys on the A71CH secure module\n"
					"\t -e\t\tpublic key\n"
					"\t -d\t\tprivate key\n"
					"\n\tinfo\t\tinformation about the tool and the environment\n"
					"\n\thelp\t\tinstruction-list and help\n\n");
}

int main(int argc, char * argv[]) {
	int c;

	if (argc <= 1) {
		tdxecc_print_help();
		return EXIT_SUCCESS;
	}

	if (strcmp(argv[1], "sign") == 0) {
		U8 signature[MAX_SIGNATURE_SIZE];
		U16 signatureLen = 0;
		U8 hash[MAX_HASH_SIZE];
		U16 hashLen = 0;

		while ((c = getopt(argc - 1, &argv[1], "h:")) != -1) {
			switch (c) {

			case 'h': /* hash */
				read_hex_value(optarg, hash, &hashLen);
				break;

			default:
				tdxecc_print_help();
				return EXIT_FAILURE;
			}
		}
		if (hashLen > 0) {
			if (tdxecc_sign_hash(hash, hashLen, signature,
					&signatureLen) == TDXECC_OK) {
				printf("Hash was signed successfully!\n");
				printf("Signature: ");
				print_hex_array(signature, signatureLen);
				printf("\n");
			} else {
				printf("Unable to sign hash.\n");
			}
		} else {
			tdxecc_print_help();
			return EXIT_FAILURE;
		}
	} else if (strcmp(argv[1], "verify") == 0) {
		U8 signature[MAX_SIGNATURE_SIZE];
		U16 signatureLen = 0;
		U8 hash[MAX_HASH_SIZE];
		U16 hashLen = 0;
		U8 eccPubKey[MAX_KEY_SIZE];
		U16 eccPubKeyLen = 0;

		while ((c = getopt(argc - 1, &argv[1], "s:h:p:")) != -1) {
			switch (c) {

			case 's': /* signature */
				read_hex_value(optarg, signature, &signatureLen);
				break;

			case 'h': /* hash */
				read_hex_value(optarg, hash, &hashLen);
				break;

			case 'p': /* public key */
				read_hex_value(optarg, eccPubKey, &eccPubKeyLen);
				break;

			default:
				tdxecc_print_help();
				return EXIT_FAILURE;
			}
		}
		if (signatureLen > 0 && hashLen > 0 && eccPubKeyLen > 0) {
			if (tdxecc_verify_signature(eccPubKey, eccPubKeyLen, hash, hashLen,
					signature, signatureLen) == TDXECC_OK) {
				printf("Signature was verified successfully!\n");
			} else {
				printf("Unable to verify signature.\n");
			}
		} else {
			tdxecc_print_help();
			return EXIT_FAILURE;
		}
	} else if (strcmp(argv[1], "overwritekeys") == 0) {
		U8 eccPubKey[MAX_KEY_SIZE];
		U16 eccPubKeyLen = 0;
		U8 eccPrivKey[MAX_KEY_SIZE];
		U16 eccPrivKeyLen = 0;

		while ((c = getopt(argc - 1, &argv[1], "e:d:")) != -1) {
			switch (c) {

			case 'e': /* public key */
				read_hex_value(optarg, eccPubKey, &eccPubKeyLen);
				break;

			case 'd': /* private key */
				read_hex_value(optarg, eccPrivKey, &eccPrivKeyLen);
				break;

			default:
				tdxecc_print_help();
				return EXIT_FAILURE;
			}
		}
		if (eccPubKeyLen > 0 && eccPrivKeyLen > 0) {
			if (tdxecc_overwrite_keys(eccPubKey, eccPubKeyLen, eccPrivKey,
					eccPrivKeyLen) == TDXECC_OK) {
				printf("New keys set successfully!\n");
			} else {
				printf("Unable to set new keys.\n");
			}
		} else {
			tdxecc_print_help();
			return EXIT_FAILURE;
		}
	} else if (strcmp(argv[1], "info") == 0) {
		tdxecc_print_info();
		return EXIT_SUCCESS;
	} else {
		tdxecc_print_help();
		return EXIT_FAILURE;
	}
}
