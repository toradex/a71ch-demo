# A71CH Library
# Not to be invoked directly

list(APPEND GEN_SOURCES
    ../libCommon/scp/scp.c
    ../libCommon/infra/sm_apdu.c
    ../libCommon/infra/sm_errors.c
    ../libCommon/infra/global_platf.c
    ../libCommon/hostCrypto/hcAsn.c
    ../api/src/ax_crypto_rng.c
    ../api/src/ax_util.c
    ../platform/generic/sm_printf.c
    ../platform/generic/sm_timer.c    
)

list(APPEND A71CH_SOURCES
    src/a71ch_com.c
    src/a71ch_crypto_derive.c
    src/a71ch_crypto_ecc.c
    src/a71ch_module.c
    src/a71ch_sst.c
    ../libCommon/infra/a71_debug.c
)

list(APPEND A71CH_SCP_SOURCES 
    ../api/src/ax_scp.c
    ../libCommon/hostCrypto/axHostCryptoOpenSSL.c
    ../libCommon/hostCrypto/HostCryptoAPIOpenSSL.c
)

list(APPEND A71CH_PUBLIC_HEADERS
    inc/a71ch_api.h
    inc/a71ch_const.h
    inc/a71ch_util.h
    ../api/inc/ax_api.h
    ../api/inc/ax_common.h
    ../api/inc/ax_scp.h
    ../libCommon/hostCrypto/axHostCrypto.h
    ../libCommon/hostCrypto/HostCryptoAPI.h
    ../libCommon/infra/global_platf.h
    ../libCommon/hostCrypto/hcAsn.h
    ../libCommon/infra/sm_apdu.h
    ../libCommon/infra/sm_types.h
    ../libCommon/infra/sm_errors.h
    ../libCommon/scp/scp.h
    ../libCommon/smCom/smCom.h
    ../libCommon/smCom/apduComm.h
    ../libCommon/infra/a71_debug.h
)

if(USE_SMCOM_SOCKET)
    list(APPEND SM_COM_SOURCES
        ../libCommon/smCom/smComSocket_linux.c
    )
    list(APPEND A71CH_PUBLIC_HEADERS
        ../libCommon/smCom/smComSocket.h
    )    
endif(USE_SMCOM_SOCKET)

if(USE_SMCOM_I2C)
    list(APPEND SM_COM_SOURCES
        ../libCommon/smCom/sci2c.c
        ../libCommon/smCom/smComSCI2C.c
        ../platform/imx/i2c_a7.c
    )
    list(APPEND A71CH_PUBLIC_HEADERS
        ../libCommon/smCom/sci2c.h
    )
endif(USE_SMCOM_I2C)

set(A71CH_LIB A71CH_${SM_COM})

find_package(OpenSSL REQUIRED)

add_library(${A71CH_LIB} SHARED 
    ${GEN_SOURCES}
    ${A71CH_SOURCES}
    ${A71CH_SCP_SOURCES}
    ${SM_COM_SOURCES}
    ../libCommon/smCom/smCom.c
)

# Not all filesystems support links, so make library versioning optional
if(USE_LIBRARY_VERSIONING)
    # TODO: Don't hard code library version
    set_target_properties(${A71CH_LIB} PROPERTIES
        VERSION 1.4.0
        SOVERSION 1 )
endif(USE_LIBRARY_VERSIONING)

target_link_libraries(${A71CH_LIB}
    ${OPENSSL_LIBRARIES}
)

target_include_directories(${A71CH_LIB}
    PUBLIC inc
    PUBLIC ../api/inc
    PUBLIC ../libCommon/scp
    PUBLIC ../libCommon/infra
    PUBLIC ../libCommon/hostCrypto
    PUBLIC ../libCommon/smCom
    PUBLIC ../platform/inc
)

install (TARGETS ${A71CH_LIB} DESTINATION lib)

install (FILES ${A71CH_PUBLIC_HEADERS}
    DESTINATION include
)
