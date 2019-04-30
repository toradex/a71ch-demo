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
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libA71CH_.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libA71CH_.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libA71CH_.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/diego/dev/projects/A71CH/axHostSw/hostLib/a71ch/build/libA71CH_.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libA71CH_.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libA71CH_.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/local/oecore-x86_64/sysroots/x86_64-angstromsdk-linux/usr/bin/arm-angstrom-linux-gnueabi/arm-angstrom-linux-gnueabi-strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libA71CH_.so")
    endif()
  endif()
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
    )
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/diego/dev/projects/A71CH/axHostSw/hostLib/a71ch/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
