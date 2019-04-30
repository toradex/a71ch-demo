# HLSE Library
# Not to be invoked directly

list(APPEND HLSE_SOURCES
    src/A71HLSEWrapper.c  
)

list(APPEND HLSE_PUBLIC_HEADERS
	inc/HLSEAPI.h
	inc/HLSEComm.h
	inc/HLSECrypto.h
	inc/HLSEMisc.h
	inc/HLSEObjects.h
	inc/HLSETypes.h
)	

set(HLSE_LIB HLSE_A71CH_${SM_COM})

find_package(OpenSSL REQUIRED)

add_library(${HLSE_LIB} SHARED 
    ${HLSE_SOURCES}
)

# Not all filesystems support links, so make library versioning optional
if(USE_LIBRARY_VERSIONING)
    # TODO: Don't hard code library version
    set_target_properties(${HLSE_LIB} PROPERTIES
        VERSION 1.0.0
        SOVERSION 1 )
endif(USE_LIBRARY_VERSIONING)

target_link_libraries(${HLSE_LIB}
	A71CH_${SM_COM}
    ${OPENSSL_LIBRARIES}
)

# Include directories are identical to A71CH_${SM_COM}
target_include_directories(${HLSE_LIB}
    PRIVATE inc
	PRIVATE ../api/inc
    PRIVATE ../libCommon/scp
    PRIVATE ../libCommon/infra
    PRIVATE ../libCommon/hostCrypto
    PRIVATE ../libCommon/smCom
    PRIVATE ../platform/inc
)

install (TARGETS ${HLSE_LIB} DESTINATION lib)

install (FILES ${HLSE_PUBLIC_HEADERS} 
	DESTINATION include
)
