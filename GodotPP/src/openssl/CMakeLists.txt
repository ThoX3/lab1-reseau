add_library(mon_openssl_wrapper STATIC
    src/crypto_utils.cpp
)

find_package(OpenSSL REQUIRED)

target_link_libraries(mon_openssl_wrapper PRIVATE OpenSSL::Crypto)

target_include_directories(mon_openssl_wrapper PUBLIC include)
target_link_libraries(mon_openssl_wrapper PUBLIC OpenSSL::Crypto)
