#!/bin/bash

# A71CH Key provisioning script
#
# Preconditions
# - A71CH with Debug Mode attached (script can be adapted to work without Debug Mode
#   relevant key slots must then be available for writing, SCP03 must be off)
#
# Postconditions
# - A complete set of key files (*.pem) created (existing ones overwritten)
# - ECC Keys that can be provisioned in the A70CI have been injected

# TODO:
# - Create a (persistent?) reference to the Ephemeral Key Pair

# GLOBAL VARIABLES
# ----------------
A71CH_PROVISIONING_SCRIPT="0.9"

ecc_param_pem="../ecc/prime256v1.pem"
# ECC keys to be stored in A71CH
# ------------------------------
ecc_key_kp_0="../ecc/ecc_key_kp_0.pem"
ecc_key_kp_pubonly_0="../ecc/ecc_key_kp_pubonly_0.pem"
ecc_key_kp_0_ref="../eccRef/ecc_key_kp_0_ref.pem"

ecc_key_kp_1="../ecc/ecc_key_kp_1.pem"
ecc_key_kp_pubonly_1="../ecc/ecc_key_kp_pubonly_1.pem"
ecc_key_kp_1_ref="../eccRef/ecc_key_kp_1_ref.pem"

ecc_key_kp_2="../ecc/ecc_key_kp_2.pem"
ecc_key_kp_pubonly_2="../ecc/ecc_key_kp_pubonly_2.pem"
ecc_key_kp_2_ref="../eccRef/ecc_key_kp_2_ref.pem"

ecc_key_kp_3="../ecc/ecc_key_kp_3.pem"
ecc_key_kp_pubonly_3="../ecc/ecc_key_kp_pubonly_3.pem"
ecc_key_kp_3_ref="../eccRef/ecc_key_kp_3_ref.pem"

ecc_key_pub_0="../ecc/ecc_key_pub_0.pem"
ecc_key_pub_pubonly_0="../ecc/ecc_key_pub_pubonly_0.pem"
ecc_key_pub_0_ref="../eccRef/ecc_key_pub_0_ref.pem"

ecc_key_pub_1="../ecc/ecc_key_pub_1.pem"
ecc_key_pub_1_ref="../eccRef/ecc_key_pub_1_ref.pem"

ecc_key_pub_2="../ecc/ecc_key_pub_2.pem"
ecc_key_pub_2_ref="../eccRef/ecc_key_pub_2_ref.pem"

# ECC keys used in demo applications, but not stored in A70CI
# -----------------------------------------------------------
remote_ecc_kp="../ecc/remote_ecc_kp.pem"

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

# Create ECC keys with openssl
# ----------------------------
execCommand "openssl ecparam -name prime256v1 -out ${ecc_param_pem}"
# Create keys
execCommand "openssl ecparam -in ${ecc_param_pem} -genkey -noout -out ${ecc_key_kp_0}"
execCommand "openssl ec -in ${ecc_key_kp_0} -pubout -out ${ecc_key_kp_pubonly_0}"
execCommand "openssl ecparam -in ${ecc_param_pem} -genkey -noout -out ${ecc_key_kp_1}"
execCommand "openssl ec -in ${ecc_key_kp_1} -pubout -out ${ecc_key_kp_pubonly_1}"
execCommand "openssl ecparam -in ${ecc_param_pem} -genkey -noout -out ${ecc_key_kp_2}"
execCommand "openssl ec -in ${ecc_key_kp_2} -pubout -out ${ecc_key_kp_pubonly_2}"
execCommand "openssl ecparam -in ${ecc_param_pem} -genkey -noout -out ${ecc_key_kp_3}"
execCommand "openssl ec -in ${ecc_key_kp_3} -pubout -out ${ecc_key_kp_pubonly_3}"

execCommand "openssl ecparam -in ${ecc_param_pem} -genkey -noout -out ${ecc_key_pub_0}"
execCommand "openssl ec -in ${ecc_key_pub_0} -pubout -out ${ecc_key_pub_pubonly_0}"
execCommand "openssl ecparam -in ${ecc_param_pem} -genkey -noout -out ${ecc_key_pub_1}"
execCommand "openssl ecparam -in ${ecc_param_pem} -genkey -noout -out ${ecc_key_pub_2}"

execCommand "openssl ecparam -in ${ecc_param_pem} -genkey -noout -out ${remote_ecc_kp}"


# Connect with A71CH, debug reset A71CH and insert keys
# -----------------------------------------------------
execCommand "${probeExec} debug reset"
execCommand "${probeExec} set pair -x 0 -k ${ecc_key_kp_0}"
execCommand "${probeExec} refpem -c 10 -x 0 -k ${ecc_key_kp_0} -r ${ecc_key_kp_0_ref}"
execCommand "${probeExec} set pair -x 1 -k ${ecc_key_kp_1}"
execCommand "${probeExec} refpem -c 10 -x 1 -k ${ecc_key_kp_1} -r ${ecc_key_kp_1_ref}"
execCommand "${probeExec} set pair -x 2 -k ${ecc_key_kp_2}"
execCommand "${probeExec} refpem -c 10 -x 2 -k ${ecc_key_kp_2} -r ${ecc_key_kp_2_ref}"
execCommand "${probeExec} set pair -x 3 -k ${ecc_key_kp_3}"
execCommand "${probeExec} refpem -c 10 -x 3 -k ${ecc_key_kp_3} -r ${ecc_key_kp_3_ref}"

execCommand "${probeExec} set pub -x 0 -k ${ecc_key_pub_0}"
execCommand "${probeExec} refpem -c 20 -x 0 -k ${ecc_key_pub_0} -r ${ecc_key_pub_0_ref}"
execCommand "${probeExec} set pub -x 1 -k ${ecc_key_pub_1}"
execCommand "${probeExec} refpem -c 20 -x 1 -k ${ecc_key_pub_1} -r ${ecc_key_pub_1_ref}"
execCommand "${probeExec} set pub -x 2 -k ${ecc_key_pub_2}"
execCommand "${probeExec} refpem -c 20 -x 2 -k ${ecc_key_pub_2} -r ${ecc_key_pub_2_ref}"

execCommand "${probeExec} info pair"
execCommand "${probeExec} info pub"
