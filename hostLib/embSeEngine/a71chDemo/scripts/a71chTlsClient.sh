#!/bin/bash

# Purpose:
# - Illustrate using a provisioned A71CH (Keypair & Client Certificate) to establish a TLS link to server
# Preconditions
# - Invoke from directory where this script is stored
# - Properly prepare Secure Element (invoke e.g. tlsPrepareClient.sh)
# Postconditions
# - 

# GLOBAL VARIABLES
# ----------------

OPENSSL_CONF_A71CH=/etc/ssl/opensslA71CH_i2c.cnf
OPENSSL_A71CH="openssl"
A71CH_TLS_CLIENT="./a71chTlsClient"

A71CH_TLS_CLIENT_DEMO_SCRIPT="0.9"

#
# Test specific files and directories
# -----------------------------------
caCert="../a71chDemo/ecc/tls_rootca.cer"
clientCert="index_0"
clientKey="../a71chDemo/eccRef/tls_client_key_ref.pem"


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
echo "A71CH supported TLS Client script (Rev.${A71CH_TLS_CLIENT_DEMO_SCRIPT})."
if [ $# -ne 1 ]; then
	echo "Usage: a71chTlsClient.sh <ipAddress>"
    echo "  Note: port 8080 is assumed"
	exit 1
else
	address_port=${1}:8080
fi

# 
# ----------------------------------------
cd ../../a71chEx

echo "Connecting to ${address_port}..."

gExecMsg="** Establishing connection over TLS to server using A71CH **"
export OPENSSL_CONF=${OPENSSL_CONF_A71CH}
execCommand "${A71CH_TLS_CLIENT}" ${address_port} ${caCert} ${clientKey}

unset OPENSSL_CONF

echo "** Demo completed successfully **"
