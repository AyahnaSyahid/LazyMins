# ------------------------------------------------------------------
# OpenSSL
#
# Karena kamu punya static library OpenSSL sendiri (dari ClamWin),
# ada 2 opsi:
#
# OPSI A - pakai find_package bawaan CMake (kalau OpenSSL ter-install
#          secara standar / lewat OPENSSL_ROOT_DIR):
#
#     cmake -DOPENSSL_ROOT_DIR=/path/ke/openssl -B build
#
# OPSI B - kalau kamu punya file .a/.lib manual dari ClamWin, override
#          manual seperti ini (paling aman untuk static lib pihak ketiga):
#
#     cmake -DOPENSSL_CUSTOM_INCLUDE=/path/include \
#           -DOPENSSL_CUSTOM_SSL_LIB=/path/libssl.a \
#           -DOPENSSL_CUSTOM_CRYPTO_LIB=/path/libcrypto.a \
#           -B build
#
# Default di bawah ini pakai OPSI A (find_package), dengan fallback pesan
# error yang jelas kalau tidak ketemu.
# ------------------------------------------------------------------

if (WIN32)
    set(OPENSSL_CUSTOM_INCLUDE "${CMAKE_SOURCE_DIR}/static_openssl_3.5/include" CACHE PATH "" FORCE)
    set(OPENSSL_CUSTOM_SSL_LIB "${CMAKE_SOURCE_DIR}/static_openssl_3.5/lib/mingw/x64/libssl.a" CACHE FILEPATH "" FORCE)
    set(OPENSSL_CUSTOM_CRYPTO_LIB "${CMAKE_SOURCE_DIR}/static_openssl_3.5/lib/mingw/x64/libcrypto.a" CACHE FILEPATH "" FORCE)
endif()

if(OPENSSL_CUSTOM_INCLUDE AND OPENSSL_CUSTOM_SSL_LIB AND OPENSSL_CUSTOM_CRYPTO_LIB)
    set(OPENSSL_INCLUDE_DIR ${OPENSSL_CUSTOM_INCLUDE})
    set(OPENSSL_SSL_LIBRARY ${OPENSSL_CUSTOM_SSL_LIB})
    set(OPENSSL_CRYPTO_LIBRARY ${OPENSSL_CUSTOM_CRYPTO_LIB})
    set(OPENSSL_LIBRARIES ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})
    message(STATUS "Memakai OpenSSL custom (ClamWin static lib): ${OPENSSL_INCLUDE_DIR}")
else()
    set(OPENSSL_USE_STATIC_LIBS ON)
    find_package(OpenSSL REQUIRED)
    set(OPENSSL_LIBRARIES OpenSSL::SSL OpenSSL::Crypto)
endif()

# ------------------------------------------------------------------
# Static library: license_helper
# ------------------------------------------------------------------
add_library(license_helper STATIC
    src/license_helper.cpp
)
target_include_directories(license_helper PUBLIC
    include
    ${OPENSSL_INCLUDE_DIR}
)
target_link_libraries(license_helper PUBLIC ${OPENSSL_LIBRARIES})

# Kalau target Windows + link OpenSSL static, biasanya perlu ws2_32/crypt32
if(WIN32)
    target_link_libraries(license_helper PUBLIC ws2_32 crypt32)
endif()

# ------------------------------------------------------------------
# Semua executable dikumpulkan di <build>/tools/
# ------------------------------------------------------------------
set(TOOLS_OUTPUT_DIR ${CMAKE_BINARY_DIR}/tools)

# ------------------------------------------------------------------
# Executable: keygen
# ------------------------------------------------------------------
add_executable(keygen ../../tools/keygen.cpp)
target_include_directories(keygen PRIVATE ${OPENSSL_INCLUDE_DIR})
target_link_libraries(keygen PRIVATE ${OPENSSL_LIBRARIES})
if(WIN32)
    target_link_libraries(keygen PRIVATE ws2_32 crypt32)
endif()
set_target_properties(keygen PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TOOLS_OUTPUT_DIR})

# ------------------------------------------------------------------
# Executable demo (opsional, contoh pemakaian library)
# ------------------------------------------------------------------
add_executable(sign_demo ../../tools/sign_demo.cpp)
target_link_libraries(sign_demo PRIVATE license_helper)
set_target_properties(sign_demo PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TOOLS_OUTPUT_DIR})

add_executable(verify_demo ../../tools/verify_demo.cpp)
target_link_libraries(verify_demo PRIVATE license_helper)
set_target_properties(verify_demo PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TOOLS_OUTPUT_DIR})

add_executable(hwid ../../tools/hwid.cpp)
target_link_libraries(hwid PRIVATE license_helper)
set_target_properties(hwid PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TOOLS_OUTPUT_DIR})
