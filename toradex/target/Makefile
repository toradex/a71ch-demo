# Invocation
#  make -f Makefile_a71chTdx platf=imx
#
#  2018-09-07: Modified to link against pre-installed libraries in /usr/local/lib
#
#  PRECONDITION
#         A71CH shared library (libA71CH_i2c.so.1.4.0) HLSE shared library (libHLSE_A71CH_i2c.so.1.0.0)
#         and associated header files is installed in a system standard location
#           (1) On iMX6UL native: e.g. /usr/local/lib and /usr/local/include respectively
#           (2) When cross-compiling, build the shared libraries with make/cmake. 
#               Install in the cross-compilation system root file system as follows:
#
#               make
#               make DESTDIR=$SDKTARGETSYSROOT install
#               (echo $DESTDIR -> /opt/fsl-imx-fb/4.9.11-1.0.0/sysroots/cortexa7hf-neon-poky-linux-gnueabi)
#
#  NOTE-1 
#         When using cross compilation (and facing issues with linking to Engine) invoke with additional parameter 
#           make -f Makefile_a71chTdx platf=imx
# 
#  NOTE-2
#         Ensure the OpenSSL engine is linked against the same share library as the application
#
TARGET      = a71chtdx 
OBJECTS     = a71chtdx.o

DFLAGS = -DI2C
RM = rm

LDFLAGS	+= -L../hostLib/build/a71ch -L../hostLib/build/api/ -L../linux -le2a71chi2c -lA71CH_i2c -lHLSE_A71CH_i2c -ldl -lssl -lcrypto

CINCS = -I../hostLib/libCommon/infra \
	-I../hostLib/build/a71ch \
	-I../hostLib/build/a71ch \
 	-I../hostLib/a71ch/inc \
	-I../hostLib/embSeEngine/inc \
	-I../hostLib/api/inc \
	-I../hostLib/libCommon/scp \
	-I../hostLib/libCommon/smCom 

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
