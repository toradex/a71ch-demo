#!/bin/bash

rootca_key="../ecc/tls_rootca_key.pem"
rootca_cer="../ecc/tls_rootca.cer"

client_key="../ecc/tls_client_key.pem"
client_key_pub="../ecc/tls_client_key_pub.pem" # Contains public key only
client_csr="../ecc/tls_client.csr"
client_cer="../ecc/tls_client.cer"

server_key="../ecc/tls_server_key.pem"
server_csr="../ecc/tls_server.csr"
server_cer="../ecc/tls_server.cer"

rootca_srl=".srl"

ecc_param_pem="../ecc/prime256v1.pem"

echo "Prepare ECC TLS credentials (Client side material)"

if [ ! -e ${ecc_param_pem} ]; then
	echo "Creating ECC parameter file: ${ecc_param_pem} for prime256v1"
	openssl ecparam -name prime256v1 -out ${ecc_param_pem}
else
	echo "ECC parameter file ${ecc_param_pem} already exists"	
fi

# Conditionally create CA
if [ ! -e ${rootca_key} ]; then
	echo ">> Creating new RootCA key (${rootca_key}) and RootCA certificate (${rootca_cer})"
	openssl ecparam -in ${ecc_param_pem} -genkey -noout -out ${rootca_key}
	openssl ec -in ${rootca_key} -text -noout
	echo ">> Creating new - selfsigned - RootCA (${rootca_cer}) certificate"
	openssl req -x509 -new -nodes -key ${rootca_key} \
	  -subj "/C=BE/ST=VlaamsBrabant/L=Leuven/O=NXP-Demo-CA/OU=Demo-Unit/CN=demoCA/emailAddress=demoCA@nxp" \
	  -days 2800 -out ${rootca_cer}
	openssl x509 -in ${rootca_cer} -text -noout
else
	if [ ! -e ${rootca_cer} ]; then
	    echo ">> Creating new - selfsigned - RootCA (${rootca_cer}) certificate"
		openssl req -x509 -new -nodes -key ${rootca_key} \
			-subj "/C=BE/ST=VlaamsBrabant/L=Leuven/O=NXP-Demo-CA/OU=Demo-Unit/CN=demoCA/emailAddress=demoCA@nxp" \
			-days 2800 -out ${rootca_cer}
		openssl x509 -in ${rootca_cer} -text -noout			
	else
		echo ">> RootCA key (${rootca_key}) and RootCA certificate (${rootca_cer}) already exist"
	fi
fi

# Conditionally create client key
if [ ! -e ${client_key}  ]; then
	echo ">> Creating client key: ${client_key}"
	openssl ecparam -in ${ecc_param_pem} -genkey -noout -out ${client_key}
	openssl ec -in ${client_key} -text -noout
else
	echo ">> ${client_key} already exists"			
fi

# Create a client key pem file containing ONLY the public key
echo ">> Creating client key with public key only; typically used by verifier: ${client_key_pub}"	
openssl ec -in ${client_key} -pubout -out ${client_key_pub}

echo ">> Now create CSR"
openssl req -new -key ${client_key} -subj "/C=BE/O=NXPDemo/OU=Unit/CN=ECC-demo-client" -out ${client_csr}
openssl req -in ${client_csr} -text -noout


# Always create a CA signed client certificate
if [ -e ${rootca_key} ] && [ -e ${rootca_cer} ]; then
	# echo "CA cert and key present"
	if [ -e ${rootca_srl} ]; then
		echo ">> ${rootca_srl} already exists, use it"
		x509_serial="-CAserial ${rootca_srl}"
	else
		echo ">> no ${rootca_srl} found, create it"
		x509_serial="-CAcreateserial"
	fi
	openssl x509 -req -sha256 -days 2800 -in ${client_csr} ${x509_serial} -CA ${rootca_cer} -CAkey ${rootca_key} -out ${client_cer}
	openssl x509 -in ${client_cer} -text -noout
else
	echo "Did not find CA cert and/or CA key pair: Fatal error"
	exit -1
fi

echo ">> Client certificate and key available for use"
echo ">> ********************************************"
echo " "
echo "Prepare ECC TLS credentials (Server side material)"

# Conditionally create server key
if [ ! -e ${server_key}  ]; then
	echo ">> Creating server key: ${server_key}"
	openssl ecparam -name prime256v1 -out ${ecc_param_pem}
	openssl ecparam -in ${ecc_param_pem} -genkey -noout -out ${server_key}
	openssl ec -in ${server_key} -text -noout

	openssl req -new -key ${server_key} -subj "/C=BE/O=NXPDemo/OU=Unit/CN=ECC-demo-server" -out ${server_csr}
else
	echo ">> ${server_key} already exists"
fi

echo ">> Create CSR anew"
openssl req -new -key ${server_key} -subj "/C=BE/O=NXPDemo/OU=Unit/CN=ECC-demo-server" -out ${server_csr}
openssl req -in ${server_csr} -text -noout

# Always create a CA signed server certificate
if [ -e ${rootca_key} ] && [ -e ${rootca_cer} ]; then
	# echo "CA cert and key present"
	if [ -e ${rootca_srl} ]; then
		echo ">> ${rootca_srl} already exists, use it"
		x509_serial="-CAserial ${rootca_srl}"
	else
		echo ">> no ${rootca_srl} found, create it"
		x509_serial="-CAcreateserial"
	fi
	openssl x509 -req -sha256 -days 2800 -in ${server_csr} ${x509_serial} -CA ${rootca_cer} -CAkey ${rootca_key} -out ${server_cer}
	openssl x509 -in ${server_cer} -text -noout
else
	echo "Did not find CA cert and/or CA key pair: Fatal error"
	exit -1
fi

echo ">> Server certificate and keys available for use"
echo ">> *********************************************"
