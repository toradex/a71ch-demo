# Date: 2018-06-28
# Toplevel CMakeLists.txt file
# Creates 
# - a71ch & hlse shared library
# - config tool, a71ch and hlse example programs linking to shared libraries
# Supports socket on PC/IMX or i2c on IMX

# Create the build_* directories at the same level as the hostLib directory

# To enable USE_SMCOM_SOCKET, invoke like
# cd build_socket
# cmake -DUSE_SMCOM_SOCKET=ON ../hostLib

# To enable USE_SMCOM_I2C, invoke like
# cd build_i2c
# cmake -DUSE_SMCOM_I2C=ON ../hostLib

# To cross-compile for iMX (and target SCI2C)
# source /opt/fsl-imx-fb/4.9.11-1.0.0/environment-setup-cortexa7hf-neon-poky-linux-gnueabi
# mkdir cross_build_i2c; cd cross_build_i2c/
# cmake -DUSE_SMCOM_I2C=ON -DCMAKE_TOOLCHAIN_FILE=../hostLib/ToolchainFile_imx6.cmake ../hostLib

# To disable versioning of shared libraries
# cmake -DUSE_LIBRARY_VERSIONING=OFF (and other options as appropriate)

# Set CMAKE_INSTALL_PREFIX appropriately in case of cross-compilation (may interfere with INSTALL_RPATH..)
# As an alternative (probably better, but MUST use at install time) use 
#   make DESTDIR=<Your-Path-Prefix-Here> install

# platform related cmake issues
# 
# make install
#    Linux: In case the installed executable can not load shared library
#    Workaround (assuming A71CH specific shared libraries are installed in /usr/local/lib)
#    # ldconfig /usr/local/lib
#
#    Cygwin: In case the installed executable can not load shared library
#    Workaround (assuming A71CH specific shared libraries are installed in /usr/local/lib)
#    Put the directory containing the shared library in the path when invoking the executable
#    # PATH=/usr/local/lib:$PATH a71chConfig_socket

cmake_minimum_required(VERSION 3.5)
project(a71ch)

# Library versioning requested by default
option(USE_LIBRARY_VERSIONING "Use library versioning" ON)

# Mutually exclusive options
option(USE_SMCOM_SOCKET "Use socket as smCom implementation")
option(USE_SMCOM_I2C "Use (SC)I2C as smCom implementation")
# Enforce options are mutually exclusive
if(USE_SMCOM_SOCKET)
    if(USE_SMCOM_I2C)
        MESSAGE(FATAL_ERROR "USE_SMCOM_SOCKET can not be combined with USE_SMCOM_I2C")
    endif(USE_SMCOM_I2C)
endif(USE_SMCOM_SOCKET)

add_definitions(
    -DTGT_A71CH 
    -DLINUX -DFTR_FILE_SYSTEM -DSCP_MODE=C_MAC_C_ENC_R_MAC_R_ENC
    -DHOST_CRYPTO -DOPENSSL -DAX_CONSOLE_LOG)

if(USE_SMCOM_SOCKET)
  add_definitions(-DRJCT_SOCKET)
  set(SM_COM "socket")
endif(USE_SMCOM_SOCKET)

if(USE_SMCOM_I2C)
  add_definitions(-DI2C)
  set(SM_COM "i2c")
  # TODO: Find the correct way to do this...
  add_definitions( -DPLATFORM_IMX )
endif(USE_SMCOM_I2C)

## BEGIN RPATH related settings ##
# Background on below RPATH related settings: 
# https://gitlab.kitware.com/cmake/community/wikis/doc/cmake/RPATH-handling

# set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib") # Using the alternative below

# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# the RPATH to be used when installing, but only if it's not a system directory
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
    SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
ENDIF("${isSystemDir}" STREQUAL "-1")

## NOTE: In case it still doesn't work, explicitly set the shared library search path, e.g.
## ldconfig /usr/local/lib (or whatever library install path selected)

## END RPATH related settings ##

## Don't do an in-source build
## ###########################
if(${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR})
    message( "  In-source builds are not allowed." )
    message( "Create build directories at the same hierarchical level as the 'hostLib' directory, e.g." )
    message( "  mkdir build_i2c" )
    message( "  cd build_i2c" )
    message( "  cmake -DUSE_SMCOM_I2C=ON ../hostLib" )
    message( "For other example invocations look at the top of the root CMakeLists.txt file" )
    message( FATAL_ERROR "  In-source builds not allowed.  Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt and CMakeFiles folder." )
endif()

## Executables to be build
## #######################
set(A71CH_CONFIG_EXE a71chConfig_${SM_COM})
set(A71CH_DEMO_EXE a71chEx_${SM_COM})
set(HLSE_DEMO_EXE a71chHlseEx_${SM_COM})

add_subdirectory(a71ch)
add_subdirectory(api)

list(APPEND A71CH_CONFIG_SOURCES
    a71ch/app/configCli.c 
    a71ch/app/configCliApdu.c 
    a71ch/app/configCliConnect.c 
    a71ch/app/configCliDebug.c 
    a71ch/app/configCliEcrt.c 
    a71ch/app/configCliErase.c 
    a71ch/app/configCliGen.c 
    a71ch/app/configCliInfo.c 
    a71ch/app/configCliInteractive_ln.c 
    a71ch/app/configCliLock.c 
    a71ch/app/configCliRcrt.c 
    a71ch/app/configCliRefpem.c 
    a71ch/app/configCliScp.c 
    a71ch/app/configCliObj.c 
    a71ch/app/configCliGet.c 
    a71ch/app/configCliScript.c 
    a71ch/app/configCliSet.c 
    a71ch/app/configCliTransport.c 
    a71ch/app/configCliWcrt.c 
    a71ch/app/configCmdApdu.c 
    a71ch/app/configCmdConnect.c 
    a71ch/app/configCmdDebug.c 
    a71ch/app/configCmdErase.c 
    a71ch/app/configCmdGen.c 
    a71ch/app/configCmdInfo.c 
    a71ch/app/configCmdLock.c 
    a71ch/app/configCmdObj.c 
    a71ch/app/configCmdGet.c 
    a71ch/app/configCmdEcrt.c 
    a71ch/app/configCmdWcrt.c 
    a71ch/app/configCmdRcrt.c 
    a71ch/app/configCmdRefpem.c 
    a71ch/app/configCmdScp.c 
    a71ch/app/configCmdSet.c 
    a71ch/app/configCmdTransport.c 
    a71ch/app/configState.c 
    a71ch/app/mainA71chConfig.c 
    tstUtil/tst_sm_util.c 
    tstUtil/tst_a71ch_util.c 
    tstUtil/axCliUtil.c 
    tstUtil/axEccRefPem.c 
    tstUtil/linenoise.c 
    tstUtil/tst_sm_time.c 
    tstUtil/tstHostCrypto.c 
    tstUtil/hkdf.c 
    tstUtil/app_boot.c
)

list(APPEND A71CH_DEMO_SOURCES
    a71ch/ex/ex_aes.c
    a71ch/ex/ex_boot.c
    a71ch/ex/ex_config.c
    a71ch/ex/ex_debug.c
    a71ch/ex/ex_ecc_nohc.c
    a71ch/ex/ex_gpstorage.c
    a71ch/ex/ex_misc.c
    a71ch/ex/ex_psk.c
    a71ch/ex/ex_scp.c
    a71ch/ex/ex_sst.c
    a71ch/ex/ex_sst_kp.c
    a71ch/ex/ex_walkthrough.c
    tstUtil/tst_sm_util.c
    tstUtil/tst_a71ch_util.c
    tstUtil/tst_sm_time.c
    tstUtil/tstHostCrypto.c
    tstUtil/hkdf.c
    tstUtil/app_boot.c
)

list(APPEND A71CH_HLSE_DEMO_SOURCES
    a71ch/ex_hlse/ex_hlse_aes.c
    a71ch/ex_hlse/ex_hlse_boot.c
    a71ch/ex_hlse/ex_hlse_cert.c
    a71ch/ex_hlse/ex_hlse_config.c
    a71ch/ex_hlse/ex_hlse_debug.c
    a71ch/ex_hlse/ex_hlse_ecc_nohc.c
    a71ch/ex_hlse/ex_hlse_gpstorage.c
    a71ch/ex_hlse/ex_hlse_misc.c
    a71ch/ex_hlse/ex_hlse_psk.c
    a71ch/ex_hlse/ex_hlse_scp.c
    a71ch/ex_hlse/ex_hlse_sst.c
    a71ch/ex_hlse/ex_hlse_sst_kp.c
    a71ch/ex_hlse/ex_hlse_walkthrough.c
    tstUtil/tst_sm_util.c
    tstUtil/tst_a71ch_util.c
    tstUtil/tst_hlse_a71ch_util.c
    tstUtil/tst_sm_time.c
    tstUtil/tstHostCrypto.c
    tstUtil/hkdf.c
    tstUtil/app_boot.c
)

find_package(OpenSSL REQUIRED)

# Config tool
add_executable(${A71CH_CONFIG_EXE}
    ${A71CH_CONFIG_SOURCES}
)

target_link_libraries(${A71CH_CONFIG_EXE}
    HLSE_A71CH_${SM_COM}
    A71CH_${SM_COM}
    ${OPENSSL_LIBRARIES}
)

target_include_directories(${A71CH_CONFIG_EXE}
    PUBLIC a71ch/app
    PUBLIC tstUtil
)

# A71CH Example
add_executable(${A71CH_DEMO_EXE}
    ${A71CH_DEMO_SOURCES}
    a71ch/ex/mainA71CH.c
)

target_link_libraries(${A71CH_DEMO_EXE}
    A71CH_${SM_COM}
    ${OPENSSL_LIBRARIES}
)

target_include_directories(${A71CH_DEMO_EXE}
    PUBLIC a71ch/ex
    PUBLIC tstUtil
)

# HLSE/A71CH Example
add_executable(${HLSE_DEMO_EXE}
    ${A71CH_HLSE_DEMO_SOURCES}
    a71ch/ex_hlse/mainA71CH_hlse.c
)

target_link_libraries(${HLSE_DEMO_EXE}
    HLSE_A71CH_${SM_COM}
    A71CH_${SM_COM}
    ${OPENSSL_LIBRARIES}
)

target_include_directories(${HLSE_DEMO_EXE}
    PUBLIC a71ch/ex
    PUBLIC tstUtil
)

install(TARGETS ${A71CH_CONFIG_EXE} ${A71CH_DEMO_EXE} ${HLSE_DEMO_EXE} 
    DESTINATION bin
)

# Header files are installed with respective shared libraries
