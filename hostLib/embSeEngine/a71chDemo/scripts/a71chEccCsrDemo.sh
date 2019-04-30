#!/bin/bash

# FILE: a71chEccCsrDemo.sh (A71CH specific demo script)
# - Illustrates the creation of a CSR with a reference key
# - Illustrates the creation of a CSR with a normal key (OpenSSL Sw implementation)
# - Use Sw OpenSSL to verify CSR
# Preconditions
# - Invoke from directory where this script is stored
# - Properly prepare Secure Element (invoke a71chPrepareEcc.sh)
# Postconditions
# - *.csr files created

# GLOBAL VARIABLES
# ----------------
OPENSSL_CONF_A71CH=/etc/ssl/opensslA71CH_i2c.cnf
OPENSSL_A71CH="openssl"

A71CH_ECC_CSR_DEMO_SCRIPT_VER="0.7"

#
# Test specific files and directories
# -----------------------------------
refCsr_0="testing_0_ref.csr"
Csr_0="testing_0.csr"

signKey0="../ecc/ecc_key_kp_0.pem"
signKeyRef0="../eccRef/ecc_key_kp_0_ref.pem"
# Verify key may only contain public key part of "../ecc/ecc_kp_0.pem" 
# when using -verify option of openssl's dgst command
verifyKeyHost0="../ecc/ecc_key_kp_pubonly_0.pem"

signKeyRef1="../eccRef/ecc_key_kp_1_ref.pem"
verifyKeyHost1="../ecc/ecc_key_kp_pubonly_1.pem"

signKeyHostPub0="../ecc/ecc_key_pub_0.pem"
verifyKeyRefPub0="../eccRef/ecc_key_pub_0_ref.pem"

signKeyHostPub1="../ecc/ecc_key_pub_1.pem"
verifyKeyRefPub1="../eccRef/ecc_key_pub_1_ref.pem"

# Set global variables to default values
gExecMsg=""
gRetValue=0

# UTILITY FUNCTIONS
# -----------------
# execCommand will stop script execution when the program executed does return gRetValue (0 by default) to the shell
execCommand () {
	local command="$*"
	echo ">> ${gExecMsg}"
	echo ">> ${command}"
	${command}
	local nRetProc="$?"
	if [ ${nRetProc} -ne ${gRetValue} ]
	then
		echo "\"${command}\" failed to run successfully, returned ${nRetProc}"
		echo "** Demo failed **"
		exit 2
	fi
	echo ""
	# Set global variables to default values
	gExecMsg=""
	gRetValue=0
} 

# --------------------------------------------------------------
# Start of program - Ensure an A71CH is connected to your system.
# --------------------------------------------------------------
echo "A71CH ECC CSR demo script (Rev.${A71CH_ECC_CSR_DEMO_SCRIPT_VER})."

# Check whether no argument is passed
# ------------------------------------------------------
if [ $# -ne 0 ]; then
	echo "No argument expected"
	exit 1
fi

# CSR Creation
# Using _0 key
# ----------------------------------------
gExecMsg="Create CSR using key (ref) ${signKeyRef0}"
export OPENSSL_CONF=${OPENSSL_CONF_A71CH}
execCommand "${OPENSSL_A71CH} req -new -key ${signKeyRef0} -subj "/C=BE/O=NXPDemo/OU=Unit/CN=ECC-CSR-demo-A70CM" -out ${refCsr_0}"
execCommand "openssl req -in ${refCsr_0} -text -noout"

unset OPENSSL_CONF
gExecMsg="Create CSR using key ${signKey0}"
execCommand "openssl req -new -key ${signKey0} -subj "/C=BE/O=NXPDemo/OU=Unit/CN=ECC-CSR-demo-SW" -out ${Csr_0}"
execCommand "openssl req -in ${Csr_0} -text -noout"

gExecMsg="Verify CSR ${refCsr_0} with SW OpenSSL"
execCommand "openssl req -text -noout -verify -in ${refCsr_0}"

gExecMsg="Verify CSR ${Csr_0} with SW OpenSSL"
execCommand "openssl req -text -noout -verify -in ${Csr_0}"

export OPENSSL_CONF=${OPENSSL_CONF_A71CH}
gExecMsg="Verify CSR ${refCsr_0} with Engine"
execCommand "${OPENSSL_A71CH} req -text -noout -verify -in ${refCsr_0}"

gExecMsg="Verify CSR ${Csr_0} with Engine"
execCommand "${OPENSSL_A71CH} req -text -noout -verify -in ${Csr_0}"

echo "** ECC CSR Demo completed successfully **"
