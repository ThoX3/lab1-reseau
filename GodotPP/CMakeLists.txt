cmake_minimum_required(VERSION 4.1)

if(NOT CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

project(GodotPP LANGUAGES CXX)

find_package(OpenSSL REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(GODOT_PROJECT_DIR "${CMAKE_SOURCE_DIR}/game")
set(GODOT_CHEAT_DIR "${CMAKE_SOURCE_DIR}/game_cheat")
set(EXPORT_DIR "${CMAKE_BINARY_DIR}/build_final")

set(GODOT_ENGINE_SRC_DIR "${CMAKE_SOURCE_DIR}/../godot-encrypted")

set(GODOT_BIN_DIR "${CMAKE_SOURCE_DIR}/.godot_bin")

if(WIN32)
    set(SCONS_PLATFORM "windows")
    set(EDITOR_BIN_NAME "godot.windows.editor.x86_64.exe")
    set(TEMPLATE_BIN_NAME "godot.windows.template_release.x86_64.exe")

    set(GODOT_OUTPUT_DIR "${GODOT_PROJECT_DIR}/bin/windows" CACHE PATH "Output Directory" FORCE)
    set(GODOT_CHEAT_OUTPUT_DIR "${GODOT_CHEAT_DIR}/bin/windows" CACHE PATH "Cheat Output Directory" FORCE)
endif()

set(GODOT_EDITOR_EXE "${GODOT_BIN_DIR}/${EDITOR_BIN_NAME}")
set(GODOT_TEMPLATE_EXE "${GODOT_BIN_DIR}/${TEMPLATE_BIN_NAME}")

add_custom_target(build_engine_editor
        COMMAND python -m SCons platform=${SCONS_PLATFORM} target=editor module_openssl_enabled=yes disable_path_overrides=no
        WORKING_DIRECTORY "${GODOT_ENGINE_SRC_DIR}"

        COMMAND ${CMAKE_COMMAND} -E make_directory "${GODOT_BIN_DIR}"

        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GODOT_ENGINE_SRC_DIR}/bin/${EDITOR_BIN_NAME}" "${GODOT_EDITOR_EXE}"

        COMMENT "Compiling Godot Editor via SCons and copying to .godot_bin..."
        USES_TERMINAL
)

add_custom_target(build_engine_template
        COMMAND python -m SCons platform=${SCONS_PLATFORM} target=template_release module_openssl_enabled=yes disable_path_overrides=no
        WORKING_DIRECTORY "${GODOT_ENGINE_SRC_DIR}"

        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GODOT_ENGINE_SRC_DIR}/bin/${TEMPLATE_BIN_NAME}" "${GODOT_TEMPLATE_EXE}"

        COMMENT "Compiling Godot Template via SCons and copying to .godot_bin..."
        USES_TERMINAL
)

add_subdirectory(externals)
add_subdirectory(src)

add_custom_target(editor
        COMMAND ${GODOT_EDITOR_EXE} -e --path "${GODOT_PROJECT_DIR}"
        DEPENDS build_engine_editor GodotPP
        COMMENT "Launching Custom Editor..."
        USES_TERMINAL
)

add_custom_target(play
        COMMAND ${GODOT_EDITOR_EXE} --path "${GODOT_PROJECT_DIR}" --editor --headless --quit || ${CMAKE_COMMAND} -E echo "Import completed."
        COMMAND ${GODOT_EDITOR_EXE} --path "${GODOT_PROJECT_DIR}"
        DEPENDS build_engine_editor GodotPP
        COMMENT "Launching game via Editor..."
        USES_TERMINAL
)

add_custom_target(cheat
        COMMAND ${GODOT_EDITOR_EXE} --path "${GODOT_CHEAT_DIR}" --editor --headless --quit || ${CMAKE_COMMAND} -E echo "Import completed."
        COMMAND ${GODOT_EDITOR_EXE} --path "${GODOT_CHEAT_DIR}"
        DEPENDS build_engine_editor GodotPP_Cheat
        COMMENT "Launching game (Cheat) via Editor..."
        USES_TERMINAL
)

add_custom_target(server
        COMMAND $<TARGET_FILE:GodotPPServer>
        DEPENDS GodotPPServer
        WORKING_DIRECTORY "$<TARGET_FILE_DIR:GodotPPServer>"
        COMMENT "Launching Standalone C++ Server..."
        USES_TERMINAL
)

add_custom_target(package
        COMMAND ${CMAKE_COMMAND} -E make_directory "${EXPORT_DIR}/bin/windows"

        COMMAND ${CMAKE_COMMAND} -E copy "${GODOT_TEMPLATE_EXE}" "${EXPORT_DIR}/GodotPP.exe"

        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:GodotPP>" "${EXPORT_DIR}/bin/windows/GodotPP.dll"

        COMMAND ${GODOT_EDITOR_EXE} --headless --path "${GODOT_PROJECT_DIR}" --export-pack "Windows Desktop" "${EXPORT_DIR}/GodotPP.pck"

        DEPENDS build_engine_editor build_engine_template GodotPP
        COMMENT "Assembly and Export completed in ${EXPORT_DIR}!"
)

add_custom_target(play_final
        COMMAND "${EXPORT_DIR}/GodotPP.exe" --verbose
        DEPENDS package
        WORKING_DIRECTORY "${EXPORT_DIR}"
        COMMENT "Launching final version (standalone)..."
        USES_TERMINAL
)
