#!/bin/bash

# Test script for a71ch configure application
#
# Preconditions
# - 
#
# Postconditions
# - 

# execCommand will stop script execution when the program executed did not return OK (i.e. 0) to the shell
execCommand () {
	local command="$*"
	echo "> ${command}"
	${command}
	local nRetProc="$?"
	if [ ${nRetProc} -ne 0 ]
	then
		echo "\"${command}\" failed, returned ${nRetProc}"
		exit 2
	fi
	echo ""
}  

# testCommand will trigger program execution, but not exit the shell when program returns 
# non zero status
testCommand () {
	local command="$*"
	echo "> ** TEST ** ${command}"
	${command}
	local nRetProc="$?"
	if [ ${nRetProc} -ne 0 ]
	then
		echo "\"${command}\" failed, returned ${nRetProc}"
		echo ">> ** TEST ** FAILED"
	fi
	echo ""
	sleep 1
}

echo "Test script for a71chConfig application"

# Check whether IP:PORT was passed as argument
if [ -z "$1" ]; then 
    ipPortVar=""
else
	ipPortVar="$1"
fi

# Evaluate the platform we're running on

platformVar=$(uname -o)
echo ${platformVar}
if [ "${platformVar}" = "Cygwin" ]; then
	echo "Running on Cygwin"
	if [ "${ipPortVar}" = "" ]; then
		# When not providing an IP:PORT parameter, we will invoke VisualStudio built exe
		# and assumme the card server is running on localhost.
		echo "Selecting Visual Studio build exe."		
		probeExec="../../../win32-vs2015/a71chConfig/Debug/a71chConfig.exe 127.0.0.1:8050"
		stringarray=($probeExec)
		if [ ! -e ${stringarray[0]} ]; then
			echo "Can't find program executable ${stringarray[0]}"
			exit 3
		fi
	else
		probeExec="../../../linux/a71chConfig_socket_native.exe ${ipPortVar}"
		stringarray=($probeExec)
		if [ ! -e ${stringarray[0]} ]; then
			echo "Can't find program executable ${stringarray[0]}"
			exit 3
		fi
	fi
else
	echo "Assume we run on Linux"
	if [ "${ipPortVar}" = "" ]; then
		probeExec="sudo ../../../linux/a71chConfig_i2c_imx"
	else
		probeExec="../../../linux/a71chConfig_socket_native ${ipPortVar}"
	fi
fi

testReset() {
	execCommand "${probeExec} debug reset"
}

testInfo () {
	execCommand "${probeExec} info device"
	execCommand "${probeExec} info cnt"
	# execCommand "${probeExec} info gp"
	execCommand "${probeExec} info pair"
	execCommand "${probeExec} info pub"
	# execCommand "${probeExec} info sym"
	execCommand "${probeExec} info all"
	execCommand "${probeExec} info status"
}

testEcc () {
	execCommand "${probeExec} set pub -x 0 -h 043802B1164C30860AC913F5F997B84158C40CFFCC1D3A4359BC22574A4FC95E628933A9E95820AD6B96A1DA106BDD5D6A8E556A78AE959C59336FE53E3A1D9ED4" 
	execCommand "${probeExec} set pub -x 1 -h 0424A889B32D8151643DFF93957561B21D5B84DA99D03AECF446CE99B9D0359D3FCCDBB4D175B1436C7C635045605F6B94C77F649652C8C6B758AA868CA5A20ED1" 
	execCommand "${probeExec} set pair -x 0 -h 04594174EFECB5CD650019358E0CD2BEFF6F1CB868CCCBA3050B4DC188672A8794A4FC140822ED97CD85B2CDF7F7DA412DA5FFE5DC44CDD69262F0064F3688FA7D \
-h 133CC4F0B176BDE5B2327BB3D8BE9CF9EA46CA9ECEBB2051388C71CF176AF25A"
	execCommand "${probeExec} set pair -x 1 -h 04D919A01B43D0DA78B2A1685B4C2FBADCFECF5FAE8C18768FE464426396E3A30211CB350EBF02DEED11FDC19B3468A900DD5529B001F66998AC7B58AAE5570EAD \
-h 74613DF12C247C7E9F58E7119BA49B2E788B69885EFD14E5BCE58269265A28E4"
	execCommand "${probeExec} gen pair -x 0"
	execCommand "${probeExec} gen pair -x 1"
	execCommand "${probeExec} lock pair -x 0"
	execCommand "${probeExec} erase pair -x 1"
	execCommand "${probeExec} lock pub -x 1"
	execCommand "${probeExec} erase pub -x 0"	
	
	execCommand "openssl ecparam -name ${ecc_curve} -out ${ecc_param_pem}"
	execCommand "openssl ecparam -in ${ecc_param_pem} -genkey -noout -out ${ecc_key_0}"
	execCommand "openssl ecparam -in ${ecc_param_pem} -genkey -noout -out ${ecc_key_1}"
	execCommand "${probeExec} set pair -x 1 -k ${ecc_key_1}"
	execCommand "${probeExec} set pub -x 0 -k ${ecc_key_0}"
}

testGp () {
	execCommand "${probeExec} set gp -h 0060 -h A4A50000EFECB5CD650019358E0CD2BEFF6F1CB868CCCBA3050B4DC188672A87"
	execCommand "${probeExec} set gp -h 0080 -h B4B50001B32D8151643DFF93957561B21D5B84DA99D03AECF446CE99B9D0359D"
	execCommand "${probeExec} set gp -h 00A0 -h C4C5000219A01B43D0DA78B2A1685B4C2FBADCFECF5FAE8C18768FE464426396"
	execCommand "${probeExec} lock gp -h 0060 -n 3"
	execCommand "${probeExec} set gp -h 00C0 -h D4D50003FFEEDDCCBBAA99887766554433221100112233445566778899AABBCC"
	execCommand "${probeExec} set gp -h 03E0 -h AABB"
	execCommand "${probeExec} set gp -h 03FF -h 77"
}

testCnt () {
	execCommand "${probeExec} set cnt -x 0 -h 00000001"
	execCommand "${probeExec} set cnt -x 1 -h 00000080"
	execCommand "${probeExec} set cnt -x 1 -h 00007000"
	execCommand "${probeExec} set cnt -x 1 -h 00600000"
	execCommand "${probeExec} set cnt -x 1 -h 50000000"
}

testSym () {
	execCommand "${probeExec} set sym -x 0 -h DBFEE9E3B276154D67F9D84CB9355456"	
	execCommand "${probeExec} set sym -x 0 -h 834996FE38D4BC9785A2C51F9FD64EF39C2C5AE0F282023F"	
	execCommand "${probeExec} set sym -x 1 -h AABBCCDDEEFF06070809A0B0B1B2B3B4"
	execCommand "${probeExec} set sym -x 1 -h 2D3631EAF469ADAF08E5B5EE97F50A14C26A6FE8DCBB39F3"
	execCommand "${probeExec} set sym -x 2 -h AABBCCDDEEFF06070809A0B0B1B2B3B4"
	execCommand "${probeExec} erase sym -x 2"
}

testSet() {
	execCommand "openssl ecparam -name ${ecc_curve} -out ${ecc_param_pem}"
	execCommand "openssl ecparam -in ${ecc_param_pem} -genkey -noout -out ${ecc_key_0}"
	execCommand "openssl ecparam -in ${ecc_param_pem} -genkey -noout -out ${ecc_key_1}"
	execCommand "openssl ecparam -in ${ecc_param_pem} -genkey -noout -out ${ecc_key_2}"
	execCommand "openssl ecparam -in ${ecc_param_pem} -genkey -noout -out ${ecc_key_3}"
	execCommand "${probeExec} set pub -x 0 -k ${ecc_key_0}"
	execCommand "${probeExec} set pub -x 1 -k ${ecc_key_1}"
	execCommand "${probeExec} set pair -x 0 -k ${ecc_key_2}"
	execCommand "${probeExec} set pair -x 1 -k ${ecc_key_3}"
	execCommand "${probeExec} info pub"
	execCommand "${probeExec} info pair"
	execCommand "openssl ec -in ${ecc_key_0} -text"
	execCommand "openssl ec -in ${ecc_key_1} -text"
	execCommand "openssl ec -in ${ecc_key_2} -text"
	execCommand "openssl ec -in ${ecc_key_3} -text"	
	execCommand "${probeExec} erase pub -x 0"
	execCommand "${probeExec} erase pub -x 1"
	execCommand "${probeExec} erase pair -x 0"
	execCommand "${probeExec} erase pair -x 1"
	execCommand "${probeExec} info pub"
	execCommand "${probeExec} info pair"
	execCommand "${probeExec} set pair -x 0 -h ${ecc_pub_hex_a} -h ${ecc_priv_hex_a}"
	execCommand "${probeExec} set pub -x 0 -h ${ecc_pub_hex_0}"
	execCommand "${probeExec} info pub"
	execCommand "${probeExec} info pair"	
}

# A71CH_CFG_KEY_IDX_MODULE_LOCK(0x00)
# configKeyModuleLockInitial, i.e. wrapkey (LEN=16):   00000000000000000000000000000000
# configKeyModuleLock (LEN=16):                        000102030405060708090A0B0C0D0E0F
# configKeyModuleLockWrapped (LEN=24): 01936CC0ECF86390F55E17ACBAA204F540A20993CDDFC95A
#
# A71CH_CFG_KEY_IDX_PRIVATE_KEYS(0x01)
# configKeyPrivateKeyInitial, i.e. wrapkey (LEN=16):   01010101010101010101010101010101
# configKeyPrivateKey (LEN=16):                        010003020504070609080B0A0D0C0F0E
# configKeyPrivateKeyWrapped (LEN=24): BF4899856C3E7B3FB11B927FE58A884CC9BB8BB25832D149

# A71CH_CFG_KEY_IDX_PUBLIC_KEYS(0x02)
# configKeyPublicKeyInitial, i.e. wrapkey (LEN=16):    02020202020202020202020202020202
# configKeyPublicKey (LEN=16):                         02030001060704050A0B08090E0F0C0D
# configKeyPublicKeyWrapped (LEN=24):  ED0ED6120432E69A6CC90BF9095964CB18D6902BDBA7DF72
#

# Public Key @ index = 0x00
# configKeyPublicKey, i.e. wrapkey (LEN=16): 02030001060704050A0B08090E0F0C0D
# PublicKey (Strip first byte before wrapping) (LEN=65):
# 04754D7679FBB239F6DE22227F7F3BFBF7D11ED32EDC6211E68FB1B38F4022D340B70B6FD11804F9EEF98BD2492263DF24663D65653B0E858FE213F9617C85882B
# PublicKeyWrapped                 (LEN=72):
# 8A9EBFA138B7FF11BB94E6761F1658E04606D3D89BD66AD638920A5248E3615085EDFD099F2C519DB720ABC8FD3C95224968FF33EEA47556CF8C8B5CDA2D1DA053C8BDA4EC5B3D47

# Public Key @ index = 0x01
# configKeyPublicKey, i.e. wrapkey (LEN=16): 02030001060704050A0B08090E0F0C0D
# PublicKey (Strip first byte before wrapping) (LEN=65):
# 04A484658C5A0837CB33B71AB7003438217E96A4C42AAD7F9AF8443A529ABE1AEF6F4D6CC14847D5AF96D247E2A7CAC66EE44BA2F58E3BD090A6A2C7EC2037921B
# PublicKeyWrapped                 (LEN=72):
# 9DE8E25FDE7F1E11678D29DF152F964A6B6BEA7E4DD8F7D944EC3A592840ACD9DEE4F7C147F2D5457E6042FA8B4A0A1470157C5A7C6978888C4B2863D0BE1524562C687A99AEFC53

# Key Pair (private key) @ index = 0x00
# configKeyPrivateKey, i.e. wrapkey (LEN=16): 010003020504070609080B0A0D0C0F0E
# PrivateKey                        (LEN=32): 1F0BD6A71673ABB1967818ED4310C7B0704BDBFAB323318C3D9245A1AD96CE83
# PrivateKeyWrapped                 (LEN=40): E79C2152A93804700D2463FB3BDD3A58F1DE30D395AED51728648DEF4004C697197B8A191F3AA693
# PublicKey                         (LEN=65): 04BF42B74FFE877C5E085EFC618D9C21927B2AC5088B52CAD141EB015E86AB1F125C8002B50E9B2A0EABA2E6F47DA89F5CCE2F12065BE9ABE00265E9326F8AB950

# Key Pair (private key) @ index = 0x01
# configKeyPrivateKey, i.e. wrapkey (LEN=16): 010003020504070609080B0A0D0C0F0E
# PrivateKey                        (LEN=32): C5F03A560F8F3F9BF2D42254656382BBEC3190D36ADD09B502F69EB7C732F874
# PrivateKeyWrapped                 (LEN=40): 6C17B13DCDDF8B98E83D5DFBBA8B4EE57867CC85404359E0D4803417EA8E75A4C204B56FE1B88683
# PublicKey                         (LEN=65): 04EE1580C83B6432A7B919C09A70DACFD705E8F2FCA1B3DB0103F841D3FF98F92E71389AB6B38536B5C774E434893A31D8458E9FA7A5FBDFC044D92E4A71E036DE

testConfigure() {
	execCommand "${probeExec} set cfg -x 0 -h 00000000000000000000000000000000"
	execCommand "${probeExec} set cfg -x 0 -h 01936CC0ECF86390F55E17ACBAA204F540A20993CDDFC95A"
	execCommand "${probeExec} set cfg -x 1 -h 01010101010101010101010101010101"
	execCommand "${probeExec} set cfg -x 1 -h BF4899856C3E7B3FB11B927FE58A884CC9BB8BB25832D149"
	execCommand "${probeExec} set cfg -x 2 -h 02020202020202020202020202020202"
	execCommand "${probeExec} set cfg -x 2 -h ED0ED6120432E69A6CC90BF9095964CB18D6902BDBA7DF72"	
	# Setting wrapped public keys
	execCommand "${probeExec} info pub"
	execCommand "${probeExec} set pub -x 0 -h 8A9EBFA138B7FF11BB94E6761F1658E04606D3D89BD66AD638920A5248E3615085EDFD099F2C519DB720ABC8FD3C95224968FF33EEA47556CF8C8B5CDA2D1DA053C8BDA4EC5B3D47"
	execCommand "${probeExec} set pub -x 1 -h 9DE8E25FDE7F1E11678D29DF152F964A6B6BEA7E4DD8F7D944EC3A592840ACD9DEE4F7C147F2D5457E6042FA8B4A0A1470157C5A7C6978888C4B2863D0BE1524562C687A99AEFC53"	
	execCommand "${probeExec} info pub"
	# Setting wrapped private keys
	execCommand "${probeExec} info pair"
	execCommand "${probeExec} set pair -x 0 -h 04BF42B74FFE877C5E085EFC618D9C21927B2AC5088B52CAD141EB015E86AB1F125C8002B50E9B2A0EABA2E6F47DA89F5CCE2F12065BE9ABE00265E9326F8AB950 \
-h E79C2152A93804700D2463FB3BDD3A58F1DE30D395AED51728648DEF4004C697197B8A191F3AA693"
	execCommand "${probeExec} set pair -x 1 -h 04EE1580C83B6432A7B919C09A70DACFD705E8F2FCA1B3DB0103F841D3FF98F92E71389AB6B38536B5C774E434893A31D8458E9FA7A5FBDFC044D92E4A71E036DE \
-h 6C17B13DCDDF8B98E83D5DFBBA8B4EE57867CC85404359E0D4803417EA8E75A4C204B56FE1B88683"
	execCommand "${probeExec} info pair"	
}

testWrap() {	

    # Cfg
	execCommand "${probeExec} set cfg -x 1 -h 01010101010101010101010101010101"
	execCommand "${probeExec} set cfg -x 1 -h 01010101010101010101010101010101 -w 01010101010101010101010101010101"
	execCommand "${probeExec} set cfg -x 2 -h 02020202020202020202020202020202"
	execCommand "${probeExec} set cfg -x 2 -h 02020202020202020202020202020202 -w 02020202020202020202020202020202"
	
	# Setting wrapped public keys
	execCommand "${probeExec} info pub"
	execCommand "${probeExec} set pub -x 0 -h 04BF42B74FFE877C5E085EFC618D9C21927B2AC5088B52CAD141EB015E86AB1F125C8002B50E9B2A0EABA2E6F47DA89F5CCE2F12065BE9ABE00265E9326F8AB950 -w 02020202020202020202020202020202"
	execCommand "${probeExec} set pub -x 1 -h 04EE1580C83B6432A7B919C09A70DACFD705E8F2FCA1B3DB0103F841D3FF98F92E71389AB6B38536B5C774E434893A31D8458E9FA7A5FBDFC044D92E4A71E036DE -w 02020202020202020202020202020202"	
	execCommand "${probeExec} info pub"
	# Setting wrapped private keys
	execCommand "${probeExec} info pair"
	execCommand "${probeExec} set pair -x 0 -h 04BF42B74FFE877C5E085EFC618D9C21927B2AC5088B52CAD141EB015E86AB1F125C8002B50E9B2A0EABA2E6F47DA89F5CCE2F12065BE9ABE00265E9326F8AB950 \
-h C5F03A560F8F3F9BF2D42254656382BBEC3190D36ADD09B502F69EB7C732F874 -w 01010101010101010101010101010101"
	execCommand "${probeExec} set pair -x 1 -h 04EE1580C83B6432A7B919C09A70DACFD705E8F2FCA1B3DB0103F841D3FF98F92E71389AB6B38536B5C774E434893A31D8458E9FA7A5FBDFC044D92E4A71E036DE \
-h 1F0BD6A71673ABB1967818ED4310C7B0704BDBFAB323318C3D9245A1AD96CE83 -w 01010101010101010101010101010101"
	execCommand "${probeExec} info pair"
	
	# From file
	execCommand "openssl ecparam -name ${ecc_curve} -out ${ecc_param_pem}"
	execCommand "openssl ecparam -in ${ecc_param_pem} -genkey -noout -out ${ecc_key_0}"
	execCommand "openssl ecparam -in ${ecc_param_pem} -genkey -noout -out ${ecc_key_1}"
	execCommand "openssl ecparam -in ${ecc_param_pem} -genkey -noout -out ${ecc_key_2}"
	execCommand "openssl ecparam -in ${ecc_param_pem} -genkey -noout -out ${ecc_key_3}"
	execCommand "${probeExec} set pub -x 0 -k ${ecc_key_0} -w 02020202020202020202020202020202"
	execCommand "${probeExec} set pub -x 1 -k ${ecc_key_1} -w 02020202020202020202020202020202"
	execCommand "${probeExec} set pair -x 0 -k ${ecc_key_2} -w 01010101010101010101010101010101"
	execCommand "${probeExec} set pair -x 1 -k ${ecc_key_3} -w 01010101010101010101010101010101"
	execCommand "${probeExec} info pub"
	execCommand "${probeExec} info pair"
	
	# Sym
	execCommand "${probeExec} set sym -x 0 -h DBFEE9E3B276154D67F9D84CB9355456"	
	execCommand "${probeExec} set sym -x 0 -h DBFEE9E3B276154D67F9D84CB9355456 -w DBFEE9E3B276154D67F9D84CB9355456"	
	execCommand "${probeExec} set sym -x 1 -h AABBCCDDEEFF06070809A0B0B1B2B3B4"
	execCommand "${probeExec} set sym -x 1 -h AABBCCDDEEFF06070809A0B0B1B2B3B4 -w AABBCCDDEEFF06070809A0B0B1B2B3B4"	
	execCommand "${probeExec} erase sym -x 0"
	execCommand "${probeExec} erase sym -x 1"
}

testWcrtRcrt(){
    # Create test.crt certificate
    execCommand "openssl req -new -newkey rsa:1024 -days 365 -nodes -x509 \
    -subj "/C=US/ST=Denial/L=Springfield/O=Dis/CN=www.example.com" \
    -keyout test.key  -out test.pem"
	# Write and read it
	execCommand "${probeExec} wcrt -x 0 -p test.pem"
	execCommand "${probeExec} rcrt -x 0 -c test.crt"
	# Create certificate from buffer and read it
	execCommand "${probeExec} wcrt -x 1 -h 30820286308201EFA003020102020900C2A9D9009754B3D4300D06092A864886\
F70D01010B0500305C310B3009060355040613025553310F300D06035504080C\
0644656E69616C3114301206035504070C0B537072696E676669656C64310C30\
0A060355040A0C034469733118301606035504030C0F7777772E6578616D706C\
652E636F6D301E170D3137303631353136313735325A170D3138303631353136\
313735325A305C310B3009060355040613025553310F300D06035504080C0644\
656E69616C3114301206035504070C0B537072696E676669656C64310C300A06\
0355040A0C034469733118301606035504030C0F7777772E6578616D706C652E\
636F6D30819F300D06092A864886F70D010101050003818D0030818902818100\
B9AD487E8C3E0E4CCA1B55959BD9231312B5B46746D8F21A69D01509298AC813\
B35AE6F96709FD0A6DEC0925A2F3428B90772E8B9D00F7F5F2895F825DD58BE0\
2F5D9611B4270BD0980C070BD89F61B0BCAAFF16DB201947A7ABB7845A9F1C49\
5FFE35EF76DC0C9BCEC87AE80470FCDC9758F595D8F0A9F56E0C2BD9F262FB3B\
0203010001A350304E301D0603551D0E041604146506780D1A0906EEB9B2752C\
D7975F078CE3C0B2301F0603551D230418301680146506780D1A0906EEB9B275\
2CD7975F078CE3C0B2300C0603551D13040530030101FF300D06092A864886F7\
0D01010B050003818100800DBC7742C034CC69AFEB999E7C1589E6B736CAE0BA\
3F100B537331AE382DAB344EF425C3C3BE5CB1AC12B0031744E1DE0441C39EBB\
246BC841672F41D92BEA9C6308CC57680847A8C82317F9B12A6A2C112C09F419\
1C1F95873FDDBB923BF99E24A5813F1ED458E2B31DC0D8B7F5368E995DD4A483\
A9B7CE73D31C7A0F7152"
	execCommand "${probeExec} rcrt -x 1"
}

testIndex(){
    # Create data compare files	
	echo  -ne '1122334455667788' > ${objData1}
	echo  -ne '8877665544332211' > ${objData2}
	echo  -ne '1234567887654321' > ${objData3}
	
	execCommand "openssl ecparam -name ${ecc_curve} -out ${ecc_param_pem}"
    # Create a certificate that is bigger than the one that will update it
    execCommand "openssl req -new -newkey ec:${ecc_param_pem} -days 365 -nodes -x509 \
        -subj "/C=US/ST=Denial/L=Springfield/O=Dis/CN=www.example.com" -keyout test.key -out ${ecc_crt_pem_bigger_size}"
    
    # Create self-signed reference certificates
    execCommand "openssl req -new -newkey ec:${ecc_param_pem} -days 365 -nodes -x509 \
        -subj "/C=BE/CN=www.example0.com" -keyout test.key -out ${ecc_crt_pem_0}"
    execCommand "openssl req -new -newkey ec:${ecc_param_pem} -days 365 -nodes -x509 \
        -subj "/C=BE/CN=www.example1.com" -keyout test.key -out ${ecc_crt_pem_1}"      
    execCommand "openssl req -new -newkey ec:${ecc_param_pem} -days 365 -nodes -x509 \
        -subj "/C=BE/CN=www.example2.com" -keyout test.key -out ${ecc_crt_pem_2}"
    execCommand "openssl req -new -newkey ec:${ecc_param_pem} -days 365 -nodes -x509 \
        -subj "/C=BE/CN=www.example3.com" -keyout test.key -out ${ecc_crt_pem_3}"
    execCommand "openssl req -new -newkey ec:${ecc_param_pem} -days 365 -nodes -x509 \
        -subj "/C=BE/O=TestingALongerCertificateAtEndOfCertificateStore/CN=www.example3b_longer_name.com" -keyout test.key -out ${ecc_crt_pem_3_b}"

	# Write 3 objects
	execCommand "${probeExec} obj write -x 0 -h 1122334455667788"
	execCommand "${probeExec} obj write -x 1 -h 8877665544332211"
	execCommand "${probeExec} obj write -x 2 -h 1234567887654321"
		
    # Now store all certificates; 
    execCommand "${probeExec} wcrt -x 0 -p ${ecc_crt_pem_0}"
    execCommand "${probeExec} wcrt -x 1 -p ${ecc_crt_pem_1}"
    execCommand "${probeExec} wcrt -x 2 -p ${ecc_crt_pem_2}"
    execCommand "${probeExec} wcrt -x 3 -p ${ecc_crt_pem_3}"
    
	# check by reading out whether the correct certificate was written    
    execCommand "${probeExec} rcrt -x 0 -c test.crt"
    execCommand "openssl x509 -inform DER -in test.crt -outform PEM -out test.pem"
    execCommand "diff ${ecc_crt_pem_bigger_size} test.pem"
    
    execCommand "${probeExec} rcrt -x 1 -c test_1.crt"
    execCommand "openssl x509 -inform DER -in test_1.crt -outform PEM -out test_1.pem"
    execCommand "diff ${ecc_crt_pem_1} test_1.pem"
    
    execCommand "${probeExec} rcrt -x 2 -c test_2.crt"
    execCommand "openssl x509 -inform DER -in test_2.crt -outform PEM -out test_2.pem"
    execCommand "diff ${ecc_crt_pem_2} test_2.pem"
    
    execCommand "${probeExec} rcrt -x 3 -c test_3.crt"
    execCommand "openssl x509 -inform DER -in test_3.crt -outform PEM -out test_3.pem"
    execCommand "diff ${ecc_crt_pem_3} test_3.pem"
	
	# Get and compare each object
	execCommand "${probeExec} obj get -x 0 -h 0000 -s 0008 -f data1.txt"	
	execCommand "diff ${objData1} data1.txt"
	execCommand "${probeExec} obj get -x 1 -h 0000 -s 0008 -f data2.txt"
	execCommand "diff ${objData2} data2.txt"
	execCommand "${probeExec} obj get -x 2 -h 0000 -s 0008 -f data3.txt"
	execCommand "diff ${objData3} data3.txt"
	
	# Clean up some of the tmp file
	rm -f ${objData1} ${objData2} ${objData3}
}	

testCrt(){
	execCommand "openssl ecparam -name ${ecc_curve} -out ${ecc_param_pem}"
    # Create a certificate that is bigger than the one that will update it
    execCommand "openssl req -new -newkey ec:${ecc_param_pem} -days 365 -nodes -x509 \
        -subj "/C=US/ST=Denial/L=Springfield/O=Dis/CN=www.example.com" -keyout test.key -out ${ecc_crt_pem_bigger_size}"
    
    # Create self-signed reference certificates
    execCommand "openssl req -new -newkey ec:${ecc_param_pem} -days 365 -nodes -x509 \
        -subj "/C=BE/CN=www.example0.com" -keyout test.key -out ${ecc_crt_pem_0}"
    execCommand "openssl req -new -newkey ec:${ecc_param_pem} -days 365 -nodes -x509 \
        -subj "/C=BE/CN=www.example1.com" -keyout test.key -out ${ecc_crt_pem_1}"      
    execCommand "openssl req -new -newkey ec:${ecc_param_pem} -days 365 -nodes -x509 \
        -subj "/C=BE/CN=www.example2.com" -keyout test.key -out ${ecc_crt_pem_2}"
    execCommand "openssl req -new -newkey ec:${ecc_param_pem} -days 365 -nodes -x509 \
        -subj "/C=BE/CN=www.example3.com" -keyout test.key -out ${ecc_crt_pem_3}"
    execCommand "openssl req -new -newkey ec:${ecc_param_pem} -days 365 -nodes -x509 \
        -subj "/C=BE/O=TestingALongerCertificateAtEndOfCertificateStore/CN=www.example3b_longer_name.com" -keyout test.key -out ${ecc_crt_pem_3_b}"

    # Now store all certificates; check by reading out whether the correct certificate was written
    # The first one is 'bigger' in size to guarantee it can be updated in place.
    execCommand "${probeExec} wcrt -x 0 -p ${ecc_crt_pem_bigger_size}"
    execCommand "${probeExec} wcrt -x 1 -p ${ecc_crt_pem_1}"
    execCommand "${probeExec} wcrt -x 2 -p ${ecc_crt_pem_2}"
    execCommand "${probeExec} wcrt -x 3 -p ${ecc_crt_pem_3}"
    
    execCommand "${probeExec} rcrt -x 0 -c test.crt"
    execCommand "openssl x509 -inform DER -in test.crt -outform PEM -out test.pem"
    execCommand "diff ${ecc_crt_pem_bigger_size} test.pem"
    
    execCommand "${probeExec} rcrt -x 1 -c test_1.crt"
    execCommand "openssl x509 -inform DER -in test_1.crt -outform PEM -out test_1.pem"
    execCommand "diff ${ecc_crt_pem_1} test_1.pem"
    
    execCommand "${probeExec} rcrt -x 2 -c test_2.crt"
    execCommand "openssl x509 -inform DER -in test_2.crt -outform PEM -out test_2.pem"
    execCommand "diff ${ecc_crt_pem_2} test_2.pem"
    
    execCommand "${probeExec} rcrt -x 3 -c test_3.crt"
    execCommand "openssl x509 -inform DER -in test_3.crt -outform PEM -out test_3.pem"
    execCommand "diff ${ecc_crt_pem_3} test_3.pem"
    
    execCommand "${probeExec} rcrt -x 1 -c test_1.crt"
    execCommand "openssl x509 -inform DER -in test_1.crt -outform PEM -out test_1.pem"
    execCommand "diff ${ecc_crt_pem_1} test_1.pem"
    
    # Now update the first certificate (index 0) inplace and check by reading out
    execCommand "${probeExec} ucrt -x 0 -p ${ecc_crt_pem_0}"
    execCommand "${probeExec} rcrt -x 0 -c test_0.crt"
    execCommand "openssl x509 -inform DER -in test_0.crt -outform PEM -out test_0.pem"
    execCommand "diff ${ecc_crt_pem_0} test_0.pem"

	# Read out and check the second certificate (index 1) again
    execCommand "${probeExec} rcrt -x 1 -c test_1.crt"
    execCommand "openssl x509 -inform DER -in test_1.crt -outform PEM -out test_1.pem"
    execCommand "diff ${ecc_crt_pem_1} test_1.pem"	
    
	# Delete (Erase) the fourth (index 3) certificate
    execCommand "${probeExec} ecrt -x 3"
	# Write another certificate on index 3
    execCommand "${probeExec} wcrt -x 3 -p ${ecc_crt_pem_3_b}"

	# Read all certificates out again and compare with reference
    execCommand "${probeExec} rcrt -x 0 -c test_0.crt"
    execCommand "openssl x509 -inform DER -in test_0.crt -outform PEM -out test_0.pem"
    execCommand "diff ${ecc_crt_pem_0} test_0.pem"
    
    execCommand "${probeExec} rcrt -x 1 -c test_1.crt"
    execCommand "openssl x509 -inform DER -in test_1.crt -outform PEM -out test_1.pem"
    execCommand "diff ${ecc_crt_pem_1} test_1.pem"
    
    execCommand "${probeExec} rcrt -x 2 -c test_2.crt"
    execCommand "openssl x509 -inform DER -in test_2.crt -outform PEM -out test_2.pem"
    execCommand "diff ${ecc_crt_pem_2} test_2.pem"
    
    execCommand "${probeExec} rcrt -x 3 -c test_3_b.crt"
    execCommand "openssl x509 -inform DER -in test_3_b.crt -outform PEM -out test_3_b.pem"
    execCommand "diff ${ecc_crt_pem_3_b} test_3_b.pem"
}

testCrtWithSegments(){
	execCommand "openssl ecparam -name ${ecc_curve} -out ${ecc_param_pem}"
    # Create a certificate that is bigger than the one that will update it
    execCommand "openssl req -new -newkey ec:${ecc_param_pem} -days 365 -nodes -x509 \
        -subj "/C=US/ST=Denial/L=Springfield/O=Dis/CN=www.example.com" -keyout test.key -out ${ecc_crt_pem_bigger_size}"
    
    # Create self-signed reference certificates
    execCommand "openssl req -new -newkey ec:${ecc_param_pem} -days 365 -nodes -x509 \
        -subj "/C=BE/CN=www.example0.com" -keyout test.key -out ${ecc_crt_pem_0}"
    execCommand "openssl req -new -newkey ec:${ecc_param_pem} -days 365 -nodes -x509 \
        -subj "/C=BE/CN=www.example1.com" -keyout test.key -out ${ecc_crt_pem_1}"      
    execCommand "openssl req -new -newkey ec:${ecc_param_pem} -days 365 -nodes -x509 \
        -subj "/C=BE/CN=www.example2.com" -keyout test.key -out ${ecc_crt_pem_2}"
    execCommand "openssl req -new -newkey ec:${ecc_param_pem} -days 365 -nodes -x509 \
        -subj "/C=BE/CN=www.example3.com" -keyout test.key -out ${ecc_crt_pem_3}"
    execCommand "openssl req -new -newkey ec:${ecc_param_pem} -days 365 -nodes -x509 \
        -subj "/C=BE/O=TestingALongerCertificateAtEndOfCertificateStore/CN=www.example3b_longer_name.com" -keyout test.key -out ${ecc_crt_pem_3_b}"

    # Now store all certificates; check by reading out whether the correct certificate was written
    # The first one is 'bigger' in size to guarantee it can be updated in place.
    execCommand "${probeExec} wcrt -x 0 -p ${ecc_crt_pem_bigger_size} -n 5"
    execCommand "${probeExec} wcrt -x 1 -p ${ecc_crt_pem_1} -n 5"
    execCommand "${probeExec} wcrt -x 2 -p ${ecc_crt_pem_2} -n 5"
    execCommand "${probeExec} wcrt -x 3 -p ${ecc_crt_pem_3} -n 5"
    
    execCommand "${probeExec} rcrt -x 0 -c test.crt"
    execCommand "openssl x509 -inform DER -in test.crt -outform PEM -out test.pem"
    execCommand "diff ${ecc_crt_pem_bigger_size} test.pem"
    
    execCommand "${probeExec} rcrt -x 1 -c test_1.crt"
    execCommand "openssl x509 -inform DER -in test_1.crt -outform PEM -out test_1.pem"
    execCommand "diff ${ecc_crt_pem_1} test_1.pem"
    
    execCommand "${probeExec} rcrt -x 2 -c test_2.crt"
    execCommand "openssl x509 -inform DER -in test_2.crt -outform PEM -out test_2.pem"
    execCommand "diff ${ecc_crt_pem_2} test_2.pem"
    
    execCommand "${probeExec} rcrt -x 3 -c test_3.crt"
    execCommand "openssl x509 -inform DER -in test_3.crt -outform PEM -out test_3.pem"
    execCommand "diff ${ecc_crt_pem_3} test_3.pem"
    
    execCommand "${probeExec} rcrt -x 1 -c test_1.crt"
    execCommand "openssl x509 -inform DER -in test_1.crt -outform PEM -out test_1.pem"
    execCommand "diff ${ecc_crt_pem_1} test_1.pem"
    
    # Now update the first certificate (index 0) inplace and check by reading out
    execCommand "${probeExec} ucrt -x 0 -p ${ecc_crt_pem_0}"
    execCommand "${probeExec} rcrt -x 0 -c test_0.crt"
    execCommand "openssl x509 -inform DER -in test_0.crt -outform PEM -out test_0.pem"
    execCommand "diff ${ecc_crt_pem_0} test_0.pem"

	# Read out and check the second certificate (index 1) again
    execCommand "${probeExec} rcrt -x 1 -c test_1.crt"
    execCommand "openssl x509 -inform DER -in test_1.crt -outform PEM -out test_1.pem"
    execCommand "diff ${ecc_crt_pem_1} test_1.pem"	
    
	# Delete (Erase) the fourth (index 3) certificate
    execCommand "${probeExec} ecrt -x 3"
	# Write another certificate on index 3
    execCommand "${probeExec} wcrt -x 3 -p ${ecc_crt_pem_3_b} -n 5"

	# Read all certificates out again and compare with reference
    execCommand "${probeExec} rcrt -x 0 -c test_0.crt"
    execCommand "openssl x509 -inform DER -in test_0.crt -outform PEM -out test_0.pem"
    execCommand "diff ${ecc_crt_pem_0} test_0.pem"
    
    execCommand "${probeExec} rcrt -x 1 -c test_1.crt"
    execCommand "openssl x509 -inform DER -in test_1.crt -outform PEM -out test_1.pem"
    execCommand "diff ${ecc_crt_pem_1} test_1.pem"
    
    execCommand "${probeExec} rcrt -x 2 -c test_2.crt"
    execCommand "openssl x509 -inform DER -in test_2.crt -outform PEM -out test_2.pem"
    execCommand "diff ${ecc_crt_pem_2} test_2.pem"
    
    execCommand "${probeExec} rcrt -x 3 -c test_3_b.crt"
    execCommand "openssl x509 -inform DER -in test_3_b.crt -outform PEM -out test_3_b.pem"
    execCommand "diff ${ecc_crt_pem_3_b} test_3_b.pem"
}

testTransport() {
	execCommand "${probeExec} set cfg -x 0 -h 000102030405060708090A0B0C0D0E0F"
	# Lock device
	execCommand "${probeExec} transport lock"
	execCommand "${probeExec} apdu -cmd 8091000100 -sw 9000"
	execCommand "${probeExec} transport unlock -h 000102030405060708090A0B0C0D0E0F"
	execCommand "${probeExec} info device"
}

testScript() {
	execCommand "${probeExec} script -f ${a71chConfigScript}"
	execCommand "${probeExec} script -f ${a71chConfigScriptPrepareScp03}"
	execCommand "${probeExec} script -f ${a71chConfigScriptUseScp03}"
	execCommand "${probeExec} script -f ${a71chConfigScriptBreakDownScp03}"
}

testObj() {    
    # Create data compare files	
	echo -ne '1122334455667788' > ${objData1}
	echo -ne '8877665544332211' > ${objData2}
	echo -ne '1234567887654321' > ${objData3}
	echo -ne '1122334400007788' > ${objData1PostUpdate}
	echo -ne '8877665500002211' > ${objData2PostUpdate}
	echo -ne '1234567800004321' > ${objData3PostUpdate}
	echo -ne '33440000' > ${objDataPartOfData1}
	echo -ne '66550000' > ${objDataPartOfData2}
	echo -ne '56780000' > ${objDataPartOfData3}
	
	# Write 3 objects
	execCommand "${probeExec} obj write -x 0 -h 1122334455667788"
	execCommand "${probeExec} obj write -x 1 -h 8877665544332211"
	execCommand "${probeExec} obj write -x 2 -h 1234567887654321"
	execCommand "${probeExec} obj write -x 3 -h 04BF42B74FFE877C5E085EFC618D9C21927B2AC5088B52CAD141EB015E86AB1F125C8002B50E9B2A0EABA2E6F47DA89F5CCE2F12065BE9ABE00265E9326F8AB950"
	# Get object with file type 16 bytes in a line	
	evalCommand "${probeExec} obj get -x 3 -h 0000 -s 0008 -f test16.obj -t hex_16"
	# Read object index 3 to file 
	execCommand "${probeExec} obj get -x 3 -h 0000 -s 0041 -f obj.txt"	
	# Create object at index 4 with data from file obj.txt
	execCommand "${probeExec} obj write -x 4 -f obj.txt"
	# Read object at index 4
	execCommand "${probeExec} obj get -x 4 -h 0000 -s 0041 -f obj4.txt"	
    # update object at index 4  
    execCommand "${probeExec} obj update -x 4 -h 0004 -f ${objData1}"	
	# Read object at index 4
	execCommand "${probeExec} obj get -x 4 -h 0000 -s 0008"	
	# Get and compare each object
	execCommand "${probeExec} obj get -x 0 -h 0000 -s 0008 -f data1.txt"	
	execCommand "diff ${objData1} data1.txt"
	execCommand "${probeExec} obj get -x 1 -h 0000 -s 0008 -f data2.txt"
	execCommand "diff ${objData2} data2.txt"
	execCommand "${probeExec} obj get -x 2 -h 0000 -s 0008 -f data3.txt"
	execCommand "diff ${objData3} data3.txt"
	# Update '0000' data at offset 4
	execCommand "${probeExec} obj update -x 0 -h 0004 -h 0000"
	execCommand "${probeExec} obj update -x 1 -h 0004 -h 0000"
	execCommand "${probeExec} obj update -x 2 -h 0004 -h 0000"
	# Get and compare each updated object
	execCommand "${probeExec} obj get -x 0 -h 0000 -s 0008 -f dataUpdated1.txt"
	execCommand "diff ${objData1PostUpdate} dataUpdated1.txt"
	execCommand "${probeExec} obj get -x 1 -h 0000 -s 0008 -f dataUpdated2.txt"
	execCommand "diff ${objData2PostUpdate} dataUpdated2.txt"
	execCommand "${probeExec} obj get -x 2 -h 0000 -s 0008 -f dataUpdated3.txt"
	execCommand "diff ${objData3PostUpdate} dataUpdated3.txt"
	# Get data from offset 2 size 4
	execCommand "${probeExec} obj get -x 0 -h 0002 -s 0004 -f data1FromOffset2.txt"
	execCommand "diff ${objDataPartOfData1} data1FromOffset2.txt"
	execCommand "${probeExec} obj get -x 1 -h 0002 -s 0004 -f data2FromOffset2.txt"
	execCommand "diff ${objDataPartOfData2} data2FromOffset2.txt"
	execCommand "${probeExec} obj get -x 2 -h 0002 -s 0004 -f data3FromOffset2.txt"
	execCommand "diff ${objDataPartOfData3} data3FromOffset2.txt"
	# Erase object 1 then read objects 0 and 2
	execCommand "${probeExec} obj erase -x 1"
	execCommand "${probeExec} obj get -x 0 -h 0000 -s 0008 -f dataUpdated1.txt"
	execCommand "diff ${objData1PostUpdate} dataUpdated1.txt"	
	execCommand "${probeExec} obj get -x 2 -h 0000 -s 0008 -f dataUpdated3.txt"
	execCommand "diff ${objData3PostUpdate} dataUpdated3.txt"
    # Fill up Object 0 with the initial content of Object 2
    execCommand "${probeExec} obj update -x 0 -h 0000 -h 1234567887654321"
    execCommand "${probeExec} obj get -x 0 -h 0000 -s 0008 -f _tmp_data_idx_0.txt"
    execCommand "diff ${objData3} _tmp_data_idx_0.txt"
	# Erase Object 0 and 2
	execCommand "${probeExec} obj erase -x 0"
	execCommand "${probeExec} obj erase -x 2"
	
	# Clean test files
	rm -f ${objData1}
	rm -f ${objData2}
	rm -f ${objData3}
	rm -f ${objData1PostUpdate}
	rm -f ${objData2PostUpdate}
	rm -f ${objData3PostUpdate}
	rm -f ${objDataPartOfData1}
	rm -f ${objDataPartOfData2}
	rm -f ${objDataPartOfData3}
    rm -f _tmp_data_idx_0.txt
}

testGetPub() {
    # Prepare public key and key pair 
    execCommand "${probeExec} set pub -x 0 -h 04EE1580C83B6432A7B919C09A70DACFD705E8F2FCA1B3DB0103F841D3FF98F92E71389AB6B38536B5C774E434893A31D8458E9FA7A5FBDFC044D92E4A71E036DE"
	execCommand "${probeExec} set pair -x 0 -h 04BF42B74FFE877C5E085EFC618D9C21927B2AC5088B52CAD141EB015E86AB1F125C8002B50E9B2A0EABA2E6F47DA89F5CCE2F12065BE9ABE00265E9326F8AB950 \
-h C5F03A560F8F3F9BF2D42254656382BBEC3190D36ADD09B502F69EB7C732F874"
    # Get public key from public key
    execCommand "${probeExec} get pub -c 20 -x 0 -k pub.pem"
	echo "$(<pub.pem)"	
	execCommand "${probeExec} get pub -c 10 -x 0 -k pub_pair.pem"
	echo "$(<pub_pair.pem)"
	rm -f pub.pem pub_pair.pem
}

# Global Variables
# ################
objData1="objData1.txt"
objData2="objData2.txt"
objData3="objData3.txt"
# objSetPartOfData="0000"
objData1PostUpdate="objData1PostUpdate.txt"
objData2PostUpdate="objData2PostUpdate.txt"
objData3PostUpdate="objData3PostUpdate.txt"
objDataPartOfData1="objDataPartOfData1.txt"
objDataPartOfData2="objDataPartOfData2.txt"
objDataPartOfData3="objDataPartOfData3.txt"

a71chConfigScript="a71ch_config_script.txt"
a71chConfigScriptPrepareScp03="a71ch_config_script_prepare_scp03.txt"
a71chConfigScriptUseScp03="a71ch_config_script_use_scp03.txt"
a71chConfigScriptBreakDownScp03="a71ch_config_script_breakdown_scp03.txt"

ecc_curve="prime256v1"
ecc_bits="256"
ecc_param_pem="prime256v1.pem"
ecc_key_0="keyfile_ecc_nist_256_0.pem"
ecc_key_1="keyfile_ecc_nist_256_1.pem"
ecc_key_2="keyfile_ecc_nist_256_2.pem"
ecc_key_3="keyfile_ecc_nist_256_3.pem"
ecc_key_4="keyfile_ecc_nist_256_4.pem"
ecc_key_5="keyfile_ecc_nist_256_5.pem"
ecc_key_6="keyfile_ecc_nist_256_6.pem"

ecc_crt_pem_bigger_size="ecc_certificate_bigger.pem"
ecc_crt_pem_0="ecc_certificate_0.pem"
ecc_crt_pem_1="ecc_certificate_1.pem"
ecc_crt_pem_2="ecc_certificate_2.pem"
ecc_crt_pem_3="ecc_certificate_3.pem"
ecc_crt_pem_3_b="ecc_certificate_3_b.pem"

ecc_priv_hex_a="203060cc4b433ac73dd287f1c75cd3787eac8cc7c9bd3d33f726aaab77ae27a2"
ecc_pub_hex_a="04951a8a0789747b3aa82007cd7b4cf650247fe7b039a496661af642b9608b20f120cd443e2ec997486d3b106e0d17e06bf15ec64c61303a7430a7053aed493257"
ecc_priv_hex_b="522f588fa434a8d52ff2c0c9edbc7e5d8438d567e2a8700e099ba18a634a614b"
ecc_pub_hex_b="04f5e695aafe455f8d8b129fc414f7377a477ef2627d00d1d6c42d30bdd8adfc194fe88fd55ff01a6ed3e0733bd3c7e0e7c867a7dad56fb3ddea3154ac5783edd5"
    
ecc_pub_hex_0="04a487673d1380fa4265bfdade8af30623a5fd823f61ea0b42bd42418c953460982f6422c2c87e00bff21ca3e24c1d4f32075994c190c91afaaf036cd10c7de06b"

# Demo invocations
# ################
# Issue a debug reset
testReset
testCrt
# Issue a debug reset
testReset
# Start positive test sequence
testInfo
testEcc
testSym
testGp
testCnt
testInfo
# Issue a new debug reset
testReset
testSet
testScript
# Issue a new debug reset
testReset
testConfigure
# Issue a new debug reset
testReset
testTransport
# Issue a new debug reset
testReset
testWcrtRcrt
# Issue a new debug reset
testReset
testCrt
# Issue a new debug reset
testReset
testWrap
# Issue a new debug reset
testReset
testObj
# Issue a new debug reset
testReset
testCrtWithSegments
# Issue a new debug reset
testReset
testGetPub

echo "Positive test finished successfully"
######## EOF #######
