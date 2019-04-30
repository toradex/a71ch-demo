#!/bin/bash

rootca_cer="../ecc/tls_rootca.cer"
server_key="../ecc/tls_server_key.pem"
server_cer="../ecc/tls_server.cer"

invokeServerCommand="FALSE"

if [ $# -ne 1 ]; then
	# Provide one and only one argument
    echo 'Usage: pcEccServer [ECDHE|ECDH|both|max]'
    exit 5
elif [ "${1}" == "ECDHE" ]; then
	sel_cipher="ECDHE-ECDSA-AES128-SHA"
	echo "Only support cipher ${sel_cipher}"
	invokeServerCommand="TRUE"
elif [ "${1}" == "ECDH" ]; then
	sel_cipher="ECDH-ECDSA-AES128-SHA"
	echo "Only support cipher ${sel_cipher}"
	invokeServerCommand="TRUE"		
elif [ "${1}" == "both" ]; then
	sel_cipher="ECDHE-ECDSA-AES128-SHA,ECDH-ECDSA-AES128-SHA"
	echo "Only support cipher ${sel_cipher}"
	invokeServerCommand="TRUE"
elif [ "${1}" == "max" ]; then
	sel_cipher="ECDHE-ECDSA-AES128-SHA,ECDH-ECDSA-AES128-SHA,ECDHE-ECDSA-AES128-SHA256"
	echo "Only support cipher ${sel_cipher}"
	invokeServerCommand="TRUE"
else
	echo 'Usage: tlsServer [ECDHE|ECDH|both|max]'
	exit 5
fi

echo "Ensure OPENSSL_CONF is not set to use OpenSSL engine"
echo "${OPENSSL_CONF}"

#
# Invoke openssl s_server with additional parameters for more info
# -msg   : show all protocol messages with hex dump
# -debug : print extensive debugging information including a hex dump of all traffic
#
if [ "${invokeServerCommand}" == "TRUE" ]; then
		cmd="openssl s_server -accept 8080 -no_ssl3 -named_curve prime256v1 \
-CAfile ${rootca_cer} \
-cert ${server_cer} -key ${server_key} \
-cipher ${sel_cipher} -Verify 2 -state -msg"
		echo "${cmd}"
		${cmd}
else
	echo 'This must not occur...'
	exit 6
fi

exit 0
