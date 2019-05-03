#!/bin/bash

#****************************************************************************
#
# Copyright (C) 2019 Toradex AG
# Contact: https://www.toradex.com/locations
#
# This file is part of the Toradex of the A71CH workshop demo.
#
# BSD License Usage
# Alternatively, you may use this file under the terms of the BSD license
# as follows:
#
# "Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#   * Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in
#     the documentation and/or other materials provided with the
#     distribution.
#   * Neither the name of Toradex Ag nor the names of its
#     contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
#****************************************************************************/

DEFAULT_CONTAINER_NAME="a71ch_demo_debian"

echo ""
echo "***************************************"
echo "     POTENTIAL TORADEX UPDATE TOOL     "
echo "                                       "
echo "            **                         "
echo "       ************                    "
echo "     ****************                  "
echo "    ******************                 "
echo " ********     ****    *   ****    **** "
echo " ********     ***        ******  ******"
echo "  ********   *****   *     ***     *** "
echo "    *****************                  "
echo "      **************                   "
echo "        **********                     "
echo "                                       "
echo " This is a demonstration for the the   "
echo " usage of the A71CH in combination with"
echo " the Toradex Colibri iMX6ULL and the   "
echo " new OS of Toradex called Torizon.     "
echo "                                       "
echo "***************************************"
echo ""

echo "Type in the DOCKER IMAGE ID which you want to replace by a new one, followed by [ENTER]:"
read TOREPLACE

echo "Type in the target TARBALL NAME of the DOCKER IMAGE which you want to install, followed by [ENTER]:"
read IMAGETODOWNLOAD

echo "Image to replace: $TOREPLACE"
echo "Image to download and install: $IMAGETODOWNLOAD"

#download of the file
echo "Executing the secure downoad of the requested image."
./a71chtdx -s 192.168.10.1 -p 8080 -f $IMAGETODOWNLOAD
if [ $? -ne 0 ]; then #if the secure download failed, stop the process
    echo "Secure download failed. Update process is cancelled. Old container is still available."
    exit 1
fi

#Stop and remove the running container
docker stop $DEFAULT_CONTAINER_NAME > /dev/null 2>&1 || true
docker rm $DEFAULT_CONTAINER_NAME > /dev/null 2>&1 || true

#Remove the image, which should be replaced by the new one
docker rmi $TOREPLACE > /dev/null 2>&1

#importing image
echo "Download done."
echo "Executing the import of the image into docker."
cat $IMAGETODOWNLOAD | docker import - torzion/a71chdemo:latest /dev/null 2>&1
if [ $? -ne 0 ]; then #if the import failed, stop the process
    echo "Import failed. Unfortunately, the old container and the image is already deleted."
    #delete local tar file
    rm $IMAGETODOWNLOAD
    exit 1
fi

#delete local tar file
rm $IMAGETODOWNLOAD

#run the image
echo "Import done."
echo "Creating a container out of the image and run it."
echo ""
docker run -it --privileged --entrypoint=/opt/welcome.sh --name $DEFAULT_CONTAINER_NAME -v /var/run/dbus:/var/run/dbus -v /dev:/dev torzion/a71chdemo:latest

