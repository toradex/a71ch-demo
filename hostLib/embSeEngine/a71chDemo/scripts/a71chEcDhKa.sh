#!/bin/bash

# Purpose:
# - Illustrates creating a pre-master secret with a local entity
# - Illustrates creating a pre-master secret with a remote entity
# Preconditions
# - Invoke from directory where this script is stored
# - Properly prepare Secure Element (invoke e.g. a71chPrepareEcc.sh)
# Postconditions
# - 

# GLOBAL VARIABLES
# ----------------

OPENSSL_CONF_A71CH=/etc/ssl/opensslA71CH_i2c.cnf
OPENSSL_A71CH="openssl"
A71CH_ECDH_KA="./a71chEcDhKa"

A71CH_ECDH_KA_DEMO_SCRIPT="0.8"

#
# Test specific files and directories
# -----------------------------------
# Can - currently - not be passed as arguments to ./a70ciEcDhKa
# signKeyA="../eccRef/ecc_key_kp_m_ref.pem"
# signKeyHostB="../ecc/ecc_key_pub_m.pem"
# verifyKeyRefB="../eccRef/ecc_key_pub_m_ref.pem"

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
echo "A71CH ECDH Key Agreement demo script (Rev.${A71CH_ECDH_KA_DEMO_SCRIPT})."
if [ $# -ne 0 ]; then
	echo "No arguments expected"
	exit 1
fi

# 
# ----------------------------------------
cd ../../a71chEx


gExecMsg="** ECDH - Key agreement using A71CH **"
export OPENSSL_CONF=${OPENSSL_CONF_A71CH}
execCommand "${A71CH_ECDH_KA}"

unset OPENSSL_CONF

echo "** Demo completed successfully **"
