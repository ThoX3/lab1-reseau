add_executable(GodotPPServer
    src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/openssl/src/crypto_utils.cpp
)

target_include_directories(GodotPPServer PRIVATE
    "${CMAKE_SOURCE_DIR}/../openssl/include"
)

target_link_directories(GodotPPServer PRIVATE
        ${CMAKE_SOURCE_DIR}/externals/openssl/lib
)

target_link_libraries(GodotPPServer PRIVATE
        snl
        EnTT::EnTT
        "${CMAKE_SOURCE_DIR}/../openssl/lib/VC/x64/MD/libcrypto.lib"
        "${CMAKE_SOURCE_DIR}/../openssl/lib/VC/x64/MD/libssl.lib"
)

if(WIN32)
    add_custom_command(TARGET GodotPPServer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/../openssl/bin/libcrypto-3-x64.dll"
        "$<TARGET_FILE_DIR:GodotPPServer>/libcrypto-3-x64.dll"

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/../openssl/bin/libssl-3-x64.dll"
        "$<TARGET_FILE_DIR:GodotPPServer>/libssl-3-x64.dll"

        COMMENT "Copying OpenSSL DLLs to Server build directory..."
    )
endif()
