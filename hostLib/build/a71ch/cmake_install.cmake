# Install script for directory: /home/diego/dev/projects/A71CH/axHostSw/hostLib/a71ch

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libA71CH_i2c.so.1.4.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libA71CH_i2c.so.1"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libA71CH_i2c.so"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "/usr/local/lib")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/home/diego/dev/projects/A71CH/axHostSw/hostLib/build/a71ch/libA71CH_i2c.so.1.4.0"
    "/home/diego/dev/projects/A71CH/axHostSw/hostLib/build/a71ch/libA71CH_i2c.so.1"
    "/home/diego/dev/projects/A71CH/axHostSw/hostLib/build/a71ch/libA71CH_i2c.so"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libA71CH_i2c.so.1.4.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libA71CH_i2c.so.1"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libA71CH_i2c.so"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHANGE
           FILE "${file}"
           OLD_RPATH "::::::::::::::"
           NEW_RPATH "/usr/local/lib")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/usr/local/oecore-x86_64/sysroots/x86_64-angstromsdk-linux/usr/bin/arm-angstrom-linux-gnueabi/arm-angstrom-linux-gnueabi-strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "/home/diego/dev/projects/A71CH/axHostSw/hostLib/a71ch/inc/a71ch_api.h"
    "/home/diego/dev/projects/A71CH/axHostSw/hostLib/a71ch/inc/a71ch_const.h"
    "/home/diego/dev/projects/A71CH/axHostSw/hostLib/a71ch/inc/a71ch_util.h"
    "/home/diego/dev/projects/A71CH/axHostSw/hostLib/a71ch/../api/inc/ax_api.h"
    "/home/diego/dev/projects/A71CH/axHostSw/hostLib/a71ch/../api/inc/ax_common.h"
    "/home/diego/dev/projects/A71CH/axHostSw/hostLib/a71ch/../api/inc/ax_scp.h"
    "/home/diego/dev/projects/A71CH/axHostSw/hostLib/a71ch/../libCommon/hostCrypto/axHostCrypto.h"
    "/home/diego/dev/projects/A71CH/axHostSw/hostLib/a71ch/../libCommon/hostCrypto/HostCryptoAPI.h"
    "/home/diego/dev/projects/A71CH/axHostSw/hostLib/a71ch/../libCommon/infra/global_platf.h"
    "/home/diego/dev/projects/A71CH/axHostSw/hostLib/a71ch/../libCommon/hostCrypto/hcAsn.h"
    "/home/diego/dev/projects/A71CH/axHostSw/hostLib/a71ch/../libCommon/infra/sm_apdu.h"
    "/home/diego/dev/projects/A71CH/axHostSw/hostLib/a71ch/../libCommon/infra/sm_types.h"
    "/home/diego/dev/projects/A71CH/axHostSw/hostLib/a71ch/../libCommon/infra/sm_errors.h"
    "/home/diego/dev/projects/A71CH/axHostSw/hostLib/a71ch/../libCommon/scp/scp.h"
    "/home/diego/dev/projects/A71CH/axHostSw/hostLib/a71ch/../libCommon/smCom/smCom.h"
    "/home/diego/dev/projects/A71CH/axHostSw/hostLib/a71ch/../libCommon/smCom/apduComm.h"
    "/home/diego/dev/projects/A71CH/axHostSw/hostLib/a71ch/../libCommon/infra/a71_debug.h"
    "/home/diego/dev/projects/A71CH/axHostSw/hostLib/a71ch/../libCommon/smCom/sci2c.h"
    )
endif()

