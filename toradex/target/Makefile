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

TARGET      = a71chtdx
OBJECTS     = a71chtdx.o

DFLAGS = -DI2C
RM = rm

LDFLAGS	+= -L../../hostLib/build/a71ch -L../../hostLib/build/api/ -L../../linux -le2a71chi2c -lA71CH_i2c -lHLSE_A71CH_i2c -ldl -lssl -lcrypto

CINCS = -I../../hostLib/libCommon/infra \
	-I../../hostLib/build/a71ch \
	-I../../hostLib/build/a71ch \
 	-I../../hostLib/a71ch/inc \
	-I../../hostLib/embSeEngine/inc \
	-I../../hostLib/api/inc \
	-I../../hostLib/libCommon/scp \
	-I../../hostLib/libCommon/smCom

CFLAGS    += -Wall -DLINUX -DSCP_MODE=NO_C_MAC_NO_C_ENC_NO_R_MAC_NO_R_ENC -O0 \
                -D_REENTRANT -DUSE_OPENSSL \
				-fPIC -DOPENSSL_PIC -DOPENSSL_LOAD_CONF \
				-DOPENSSL_DYNAMIC_ENGINE \
				-DTGT_A71CH -DHOST_CRYPTO -DOPENSSL

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@ echo "Generating $(TARGET)"
	@ echo "$(OBJECTS)"
	$(CC) -o $(TARGET) $(OBJECTS) $(LINCS) $(LDFLAGS) $(CINCS) $(CFLAGS) $(DFLAGS)

.SUFFIXES: .c

.c.o:
	@ echo "Compiling $<"
	@ echo "$(CC) -c $(CFLAGS) $(CINCS) $<"
	@ $(CC) -c $(DFLAGS) $(CFLAGS) $(CINCS) $<

clean:
	$(RM) $(TARGET) $(OBJECTS)

cleanall:
	$(RM) $(TARGET) $(OBJECTS)
