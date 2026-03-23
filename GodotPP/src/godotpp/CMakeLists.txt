# Define C++ GDExtension Library
add_library(GodotPP SHARED
        src/gd_example.cpp
        src/register_types.cpp
        ${CMAKE_SOURCE_DIR}/../openssl/src/crypto_utils.cpp
)
target_link_libraries(GodotPP PRIVATE
        godot::cpp
        snl
        "${CMAKE_SOURCE_DIR}/externals/openssl/lib/VC/x64/MD/libcrypto.lib"
        "${CMAKE_SOURCE_DIR}/externals/openssl/lib/VC/x64/MD/libssl.lib"
)
target_include_directories(GodotPP PUBLIC include PRIVATE 
        ${CMAKE_SOURCE_DIR}/externals/openssl/include
        ${CMAKE_SOURCE_DIR}/../openssl/include
)

target_link_directories(GodotPP PRIVATE 
        ${CMAKE_SOURCE_DIR}/externals/openssl/lib
)

# The "Install" Step (Copy Lib + Generate .gdextension)
add_custom_command(TARGET GodotPP POST_BUILD
        # Copy the binary to the Godot project bin/ folder
        COMMAND ${CMAKE_COMMAND} -E make_directory "${GODOT_OUTPUT_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:GodotPP>" "${GODOT_OUTPUT_DIR}"
        
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/externals/openssl/bin/libcrypto-3-x64.dll"
        "${GODOT_OUTPUT_DIR}/libcrypto-3-x64.dll"

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/externals/openssl/bin/libssl-3-x64.dll"
        "${GODOT_OUTPUT_DIR}/libssl-3-x64.dll"

        # BGenerate the .gdextension file from template
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/godotpp.gdextension.in"
        "${GODOT_PROJECT_DIR}/godotpp.gdextension"

        COMMENT "Installing GDExtension to ${GODOT_OUTPUT_DIR}..."
)
