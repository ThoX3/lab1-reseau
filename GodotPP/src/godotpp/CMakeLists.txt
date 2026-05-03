# ==============================================================================
# 1. LE CLIENT NORMAL (Sain)
# ==============================================================================
add_library(GodotPP SHARED
        src/gd_example.cpp
        src/register_types.cpp
        ${CMAKE_SOURCE_DIR}/src/openssl/src/crypto_utils.cpp
)
target_link_libraries(GodotPP PRIVATE
        godot::cpp
        snl
        "${CMAKE_SOURCE_DIR}/../openssl/lib/VC/x64/MD/libcrypto.lib"
        "${CMAKE_SOURCE_DIR}/../openssl/lib/VC/x64/MD/libssl.lib"
)
target_include_directories(GodotPP PUBLIC include PRIVATE
        "${CMAKE_SOURCE_DIR}/../openssl/include"
)

target_link_directories(GodotPP PRIVATE
        "${CMAKE_SOURCE_DIR}/../openssl/lib"
)

add_custom_command(TARGET GodotPP POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${GODOT_OUTPUT_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:GodotPP>" "${GODOT_OUTPUT_DIR}/$<TARGET_FILE_NAME:GodotPP>"

        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/../openssl/bin/libcrypto-3-x64.dll" "${GODOT_OUTPUT_DIR}/libcrypto-3-x64.dll"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/../openssl/bin/libssl-3-x64.dll" "${GODOT_OUTPUT_DIR}/libssl-3-x64.dll"

        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/godotpp.gdextension.in" "${GODOT_PROJECT_DIR}/godotpp.gdextension"
        COMMENT "Installing NORMAL GDExtension to ${GODOT_OUTPUT_DIR}..."
)


# ==============================================================================
# 2. LE CLIENT CHEAT (Hacker)
# ==============================================================================
add_library(GodotPP_Cheat SHARED
        src/gd_example_cheat.cpp
        src/register_types.cpp
        ${CMAKE_SOURCE_DIR}/src/openssl/src/crypto_utils.cpp
)
target_link_libraries(GodotPP_Cheat PRIVATE
        godot::cpp
        snl
        "${CMAKE_SOURCE_DIR}/../openssl/lib/VC/x64/MD/libcrypto.lib"
        "${CMAKE_SOURCE_DIR}/../openssl/lib/VC/x64/MD/libssl.lib"
)
target_include_directories(GodotPP_Cheat PUBLIC include PRIVATE
        "${CMAKE_SOURCE_DIR}/../openssl/include"
)

target_link_directories(GodotPP_Cheat PRIVATE
        "${CMAKE_SOURCE_DIR}/../openssl/lib"
)

add_custom_command(TARGET GodotPP_Cheat POST_BUILD
        # 1. On clone complètement le dossier 'game' vers 'game_cheat'
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${GODOT_PROJECT_DIR}" "${GODOT_CHEAT_DIR}"

        # 2. On s'assure que le dossier bin du clone existe
        COMMAND ${CMAKE_COMMAND} -E make_directory "${GODOT_CHEAT_OUTPUT_DIR}"

        # 3. L'INJECTION : On copie le cheat dans le clone en le renommant 'GodotPP.dll'
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:GodotPP_Cheat>" "${GODOT_CHEAT_OUTPUT_DIR}/GodotPP.dll"

        # 4. On copie les DLL OpenSSL requises
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/../openssl/bin/libcrypto-3-x64.dll" "${GODOT_CHEAT_OUTPUT_DIR}/libcrypto-3-x64.dll"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/../openssl/bin/libssl-3-x64.dll" "${GODOT_CHEAT_OUTPUT_DIR}/libssl-3-x64.dll"

        # 5. On génère le fichier de registre Godot pour le clone
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/godotpp.gdextension.in" "${GODOT_CHEAT_DIR}/godotpp.gdextension"

        COMMENT "Clonage du jeu et installation du CHEAT dans ${GODOT_CHEAT_DIR}..."
)
