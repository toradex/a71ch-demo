#!/bin/bash

# Build script for A71CH
#
# Preconditions
# -
#
# Postconditions
# -

dryRun="off"
match="off"

# execCommand will stop script execution when the program executed did not return OK (i.e. 0) to the shell
execCommand () {
    local command="$*"
    echo ">> ${command}"
    match="on"
    if [ "${dryRun}" = "off" ]; then
        ${command}
        local nRetProc="$?"
        if [ ${nRetProc} -ne 0 ]; then
            echo "\"${command}\" failed to run successfully, returned ${nRetProc}"
            exit 2
        fi
        echo ""
    fi
}

# testCommand will trigger program execution, but not exit the shell when program returns
# non zero status
testCommand () {
    local command="$*"
    echo ">> ** TEST ** ${command}"
    ${command}
    local nRetProc="$?"
    if [ ${nRetProc} -ne 0 ]; then
        echo "\"${command}\" failed to run successfully, returned ${nRetProc}"
        echo ">> ** TEST ** FAILED"
    fi
    echo ""
    sleep 1
}

echo "Script to automatically build/evaluate build success of specific A71CH solution binaries"
if [ "$#" -lt 1 ]; then
    echo "Please provide as argument one off:"
    echo "  all | A71CH | RJCT"
    exit 10
elif [ "$#" -eq 2 ]; then
    if [ $2 = "dry" ]; then
        dryRun="on"
    else
        echo "Only 'dry' is allowed as second argument"
        echo "The option 'dry' will echo the build commands to the console without executing them"
        exit 11
    fi
fi

tgt=$1
echo "Compiling ${tgt} binaries"

# Evaluate the platform we're running, detect whether a cross compilation env was set-up
# if it was do cross compilation
# The variable targetPlatform will be set to either
# - pc_linux
# - rsp_linux
# - imx_cc
# - pc_cygwin
#

platformVar=$(uname)
if [ "${platformVar}" = "Darwin" ]; then
    # OSX doesn't know 'uname -o'
    echo ${platformVar}
else
    platformVar=$(uname -o)
    hwPlatform=$(uname -i)
    hostnameVar=$(uname -n)
    echo ${platformVar}
fi
if [ "${platformVar}" = "Cygwin" ]; then
    echo "** Running and compiling for Cygwin **"
    targetPlatform="pc_cygwin"
elif [ "${platformVar}" = "Darwin" ]; then
    echo "** Running and compiling for OSX **"
    targetPlatform="osx"
elif [ "${platformVar}" = "GNU/Linux" ]; then
    if [ "${hwPlatform}" = "x86_64" ]; then
        if [ "${CROSS_COMPILE}" = "arm-angstrom-linux-gnueabi-" ]; then
            echo "** Cross compiling for imx **"
            targetPlatform="imx_cc"
        else
            echo "** Running and compiling for PC linux **"
            targetPlatform="pc_linux"
        fi
    elif [ "${hostnameVar}" = "imx6ulevk" ]; then
        echo "** Assume we run on iMX6ULEVK **"
        targetPlatform="imx_linux"
    else
        echo "** Assume we run on RspPi **"
        targetPlatform="rsp_linux"
    fi
else
    echo "Could not detect platform: exiting script"
    exit 3
fi

# Compiled binaries depend on value ${targetPlatform}
#
if [ ${targetPlatform} = "pc_linux" ]; then
    if [[ ${tgt} = "A71CH" || ${tgt} = "all" ]]; then
        execCommand "make -f Makefile_A71CH default app=A71CH conn=socket platf=native"
        execCommand "make -f Makefile_A71CH default app=A71CH_CONFIG conn=socket platf=native"
        execCommand "make -f Makefile_A71CH default app=A71CH_LIGHT conn=socket platf=native"
    fi
    if [ ${match} = "off" ]; then
        echo "Nothing to build for ${tgt}"
        exit 11
    fi
elif [ ${targetPlatform} = "pc_cygwin" ]; then
    if [[ ${tgt} = "A71CH" || ${tgt} = "all" ]]; then
        execCommand "make -f Makefile_A71CH default app=A71CH conn=socket platf=native"
        execCommand "make -f Makefile_A71CH default app=A71CH_CONFIG conn=socket platf=native"
        execCommand "make -f Makefile_A71CH default app=A71CH_LIGHT conn=socket platf=native"
        execCommand "make -f Makefile_A71CH default app=A71CH_HLSE conn=socket platf=native"
        execCommand "make -f Makefile_A71CH lib app=A71CH_LIB conn=socket platf=native"
        echo "Create a cygwin link inside directory A71CH/lib: ln -s libA71CH_socket_native.so libA71CH_socket_native.dll"
        echo "Info on cygwin linking issue: https://stackoverflow.com/questions/16154130/cygwin-g-linker-doesnt-find-shared-library"
        execCommand "make -f Makefile_A71CH dl app=A71CH_DL_EXE conn=socket platf=native"
    fi
    if [ ${match} = "off" ]; then
        echo "Nothing to build for ${tgt}"
        exit 11
    fi
elif [[ ${targetPlatform} = "imx_cc" || ${targetPlatform} = "imx_linux" ]]; then
    if [[ ${tgt} = "A71CH" || ${tgt} = "all" ]]; then
        execCommand "make -f Makefile_A71CH default app=A71CH conn=i2c platf=imx"
#        execCommand "make -f Makefile_A71CH default app=A71CH conn=socket platf=imx"
        execCommand "make -f Makefile_A71CH default app=A71CH_CONFIG conn=i2c platf=imx"
#        execCommand "make -f Makefile_A71CH default app=A71CH_CONFIG conn=socket platf=imx"
        execCommand "make -f Makefile_A71CH default app=A71CH_LIGHT conn=i2c platf=imx"
#        execCommand "make -f Makefile_A71CH default app=A71CH_LIGHT conn=socket platf=imx"
        execCommand "make -f Makefile_A71CH default app=A71CH_HLSE conn=i2c platf=imx"
# Build/Install shared libraries with cmake approach instead
#        execCommand "make -f Makefile_A71CH lib app=A71CH_LIB conn=i2c platf=imx"
#        execCommand "make -f Makefile_A71CH dl app=A71CH_DL_EXE conn=i2c platf=imx"
#        execCommand "make -f Makefile_A71CH dl app=A71CH_CONFIG_DL_EXE conn=i2c platf=imx"
#        execCommand "make -f Makefile_A71CH engine app=A71CH_ENGINE conn=i2c platf=imx"
#        execCommand "make -f Makefile_A71CH engine app=A71CH_DL_ENGINE conn=i2c platf=imx"
        execCommand "make -f Makefile_A71CH engine app=A71CH_LINKED_ENGINE conn=i2c platf=imx"		
    fi
    if [[ ${tgt} = "RJCT" || ${tgt} = "all" ]]; then
        execCommand "make -f Makefile_A71CH default app=RJCT_A71 conn=i2c platf=imx"
    fi
    if [ ${match} = "off" ]; then
        echo "Nothing to build for ${tgt}"
        exit 11
    fi
elif [ ${targetPlatform} = "osx" ]; then
    if [[ ${tgt} = "A71CH" || ${tgt} = "all" ]]; then
        echo "Support for OSX is currently limited"
        execCommand "make -f Makefile_A71CH default app=A71CH_LIGHT conn=socket platf=native"
    fi
    if [ ${match} = "off" ]; then
        echo "Nothing to build for ${tgt}"
        exit 11
    fi
else
    echo "Unreachable branch reached: Fatal script error"
    exit 4
fi

if [ "${dryRun}" = "off" ]; then
    echo "Build evaluation succeeded"
else
    echo "Build evaluation (dry run) succeeded"
fi
