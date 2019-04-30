#!/bin/bash

targetPack="A71CH_Engine.tgz"

currentDir=`pwd`

# This script must be run from the .../axHostSw/linux directory

SourceCodeRoot=".."

cd $SourceCodeRoot

# The following command prefixes axHostSw to the path of each file
# included in the archive. For this to work properly, the file or directory
# path passed as an argument must start with a '.'

tar --transform "s/^\./axHostSw/" -cvzf ${currentDir}/${targetPack} \
    ./hostLib/embSeEngine/a71chDemo/ecc \
    ./hostLib/embSeEngine/a71chDemo/eccRef \
    ./hostLib/embSeEngine/a71chDemo/scripts \
    ./hostLib/embSeEngine/a71chEx/a71chEcDhKa \
    ./hostLib/embSeEngine/a71chEx/a71chEcDhKa.c \
    ./hostLib/embSeEngine/a71chEx/a71chEcDhKa.o \
    ./hostLib/embSeEngine/a71chEx/a71chTlsClient \
    ./hostLib/embSeEngine/a71chEx/a71chTlsClient.c \
    ./hostLib/embSeEngine/a71chEx/a71chTlsClient.o \
    ./hostLib/embSeEngine/a71chEx/Makefile_a71chEcDhKa \
    ./hostLib/embSeEngine/a71chEx/Makefile_a71chTlsClient \
    ./hostLib/embSeEngine/info/opensslA71CH_i2c.cnf \
    ./linux/libe2a71chi2c.so \
    ./linux/libe2a71chi2c.so.1.0.0 \
    ./linux/a71chConfig_i2c_imx

nRetProc="$?"	
if [ ${nRetProc} -ne 0 ]; then
	echo "Failed to create ${currentDir}/${targetPack} successfully, returned ${nRetProc}"
	echo "This script must be run from the .../axHostSw/linux directory"
	exit 2
fi	
	
echo "Created: ${targetPack}" 

# done
