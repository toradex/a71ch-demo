#!/bin/bash

# Illustrates fetching random data from the A71CH
#
# Preconditions
# - Connect properly prepared A71CH (Not Tranport Locked / No SCP03 on) 
#   to system

# GLOBAL VARIABLES
# ----------------

OPENSSL_CONF_A71CH=/etc/ssl/opensslA71CH_i2c.cnf
OPENSSL_A71CH="openssl"

A71CH_RAND_DEMO_SCRIPT_VER="0.8"

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
echo "A71CH Request Random Data demo script (Rev.${A71CH_RAND_DEMO_SCRIPT_VER})."

echo "Configure OpenSSL to use A71CH"
export OPENSSL_CONF=${OPENSSL_CONF_A71CH}

nRandom="40"
gExecMsg="Request ${nRandom} byte from A71CH via OpenSSL"
execCommand "${OPENSSL_A71CH} rand -hex ${nRandom}"

nRandom="400"
gExecMsg="Request ${nRandom} byte from A71CH via OpenSSL"
execCommand "${OPENSSL_A71CH} rand -hex ${nRandom}"

nRandom="1400"
gExecMsg="Request ${nRandom} byte from A71CH via OpenSSL"
execCommand "${OPENSSL_A71CH} rand -hex ${nRandom}"

echo "** Demo completed successfully **"
