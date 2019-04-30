#!/bin/bash

# A71CH Key provisioning script for TLS demo
#
# Preconditions
# - A71CH with Debug Mode attached (script can be adapted to work without Debug Mode
#   relevant key slots must then be available for writing, SCP03 must be off)
#
# Postconditions
# - Key pair injected on Index 0
# - Ref pem created
# - Client certificate injected on index 0
# - CA certificate injected on index 1
# - Data object created on index 0 with a size of 1024 byte (data initialized at start and beginning)

# GLOBAL VARIABLES
# ----------------
A71CH_PROVISIONING_SCRIPT="0.9"

# ECC keys to be stored in A71CH
# ------------------------------
client_key="../ecc/tls_client_key.pem"
client_key_ref="../eccRef/tls_client_key_ref.pem"
client_key_pub="../ecc/tls_client_key_pub.pem" # Contains public key only

# Client certificate
client_cer="../ecc/tls_client.cer"

# Ca certificate
caCert="../ecc/tls_rootca.cer"

# UTILITY FUNCTIONS
# -----------------
# execCommand will stop script execution when the program executed did not return OK (i.e. 0) to the shell
execCommand () {
	local command="$*"
	echo ">> ${command}"
	${command}
	local nRetProc="$?"
	if [ ${nRetProc} -ne 0 ]
	then
		echo "\"${command}\" failed to run successfully, returned ${nRetProc}"
		exit 2
	fi
	echo ""
}  

# testCommand will trigger program execution, but not exit the shell when program returns 
# non zero status
testCommand () {
	local command="$*"
	echo ">> ** TEST ** ${command}"
	${command}
	local nRetProc="$?"
	if [ ${nRetProc} -ne 0 ]
	then
		echo "\"${command}\" failed to run successfully, returned ${nRetProc}"
		echo ">> ** TEST ** FAILED"
	fi
	echo ""
	# sleep 1
}

# --------------------------------------------------------------
# Start of program - Ensure an A71CH is connected to your system.
# --------------------------------------------------------------
echo "A71CH Key provisioning script (Rev.${A71CH_PROVISIONING_SCRIPT})."
echo "Executing this script will create new keys and insert these keys"
echo "in the attached A71CH secure element."

read -r -p "Are you sure? [y/N] " response
if [[ $response =~ ^([yY][eE][sS]|[yY])$ ]]
then
    echo "Continuing"
else
    echo "Halting script execution. Key files not touched. Secure Element not provisioned!"
	exit 1
fi

# Check whether IP:PORT was passed as argument
if [ -z "$1" ]; then 
    ipPortVar=""
else
	ipPortVar="$1"
fi

# Evaluate the platform we're running on and set exec path and
# command line argumemts to match platform
# This 'heuristic' can be overruled by setting the probeExec variable upfront
# 
# probeExec="sudo ../../../../linux/a71chConfig_iicbird_native"
#
if [ -z "$probeExec" ]; then
	platformVar=$(uname -o)
	echo ${platformVar}
	if [ "${platformVar}" = "Cygwin" ]; then
		echo "Running on Cygwin"
		if [ "${ipPortVar}" = "" ]; then
			# When not providing an IP:PORT parameter, we will invoke VisualStudio built exe
			# and assumme the card server is running on localhost.
			echo "Selecting Visual Studio build exe."		
			probeExec="../../../../win32-vs2015/a71chConfig/Debug/a71chConfig.exe 127.0.0.1:8050"
		else
			probeExec="../../../../linux/a71chConfig_socket_native.exe ${ipPortVar}"
		fi	
	else
		echo "Assume we run on Linux"
		if [ "${ipPortVar}" = "" ]; then
			probeExec="../../../../linux/a71chConfig_i2c_imx"
		else		
			probeExec="../../../../linux/a71chConfig_socket_native ${ipPortVar}"			
		fi
	fi
fi

# Sanity check on existence probeExec
stringarray=($probeExec)
if [ "${stringarray[0]}" = "sudo" ]; then
	if [ ! -e ${stringarray[1]} ]; then
		echo "Can't find program executable ${stringarray[1]}"
		exit 3
	fi
else
	if [ ! -e ${stringarray[0]} ]; then
		echo "Can't find program executable ${stringarray[0]}"
		exit 3
	fi
fi

# Connect with A71CH, debug reset A71CH and insert keys
# -----------------------------------------------------
execCommand "${probeExec} debug reset"
execCommand "${probeExec} set pair -x 0 -k ${client_key}"
execCommand "${probeExec} refpem -c 10 -x 0 -k ${client_key} -r ${client_key_ref}"
execCommand "${probeExec} info pair"
# By passing the option '-n 2' to the wcrt command room is created to expand 
# the certificate later on.
execCommand "${probeExec} wcrt -x 0 -p ${client_cer} -n 2"
execCommand "${probeExec} wcrt -x 1 -p ${caCert} -n 2"
execCommand "${probeExec} obj write -x 0 -n 32"
execCommand "${probeExec} obj update -x 0 -h 0000 -h 426567696E206F626A30"
execCommand "${probeExec} obj update -x 0 -h 03f8 -h 456E64206F626A30"

