cmake_minimum_required(VERSION 4.1)

# 1. Chemin relatif vers vcpkg
if(NOT CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

project(GodotPP LANGUAGES CXX)

find_package(OpenSSL REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==============================================================
# --- DÉCLARATION DES CHEMINS ---
# ==============================================================

set(GODOT_PROJECT_DIR "${CMAKE_SOURCE_DIR}/game")
set(GODOT_CHEAT_DIR "${CMAKE_SOURCE_DIR}/game_cheat")
set(EXPORT_DIR "${CMAKE_BINARY_DIR}/build_final")

# Le dossier source de ton moteur chiffré
set(GODOT_ENGINE_SRC_DIR "${CMAKE_SOURCE_DIR}/../godot-encrypted")

# Ton nouveau dossier cache local pour stocker les exécutables du moteur
set(GODOT_BIN_DIR "${CMAKE_SOURCE_DIR}/.godot_bin")

if(WIN32)
    set(SCONS_PLATFORM "windows")
    set(EDITOR_BIN_NAME "godot.windows.editor.x86_64.exe")
    set(TEMPLATE_BIN_NAME "godot.windows.template_release.x86_64.exe")

    set(GODOT_OUTPUT_DIR "${GODOT_PROJECT_DIR}/bin/windows" CACHE PATH "Dossier de sortie" FORCE)
    set(GODOT_CHEAT_OUTPUT_DIR "${GODOT_CHEAT_DIR}/bin/windows" CACHE PATH "Dossier de triche" FORCE)
endif()

# Les chemins finaux que CMake va utiliser pour lancer le jeu
set(GODOT_EDITOR_EXE "${GODOT_BIN_DIR}/${EDITOR_BIN_NAME}")
set(GODOT_TEMPLATE_EXE "${GODOT_BIN_DIR}/${TEMPLATE_BIN_NAME}")

# ==============================================================
# --- 1. COMPILATION DU MOTEUR VIA SCONS ---
# ==============================================================

# Compilation de l'Éditeur
add_custom_target(build_engine_editor
        # Lance SCons dans le dossier godot-encrypted
        COMMAND python -m SCons platform=${SCONS_PLATFORM} target=editor module_openssl_enabled=yes disable_path_overrides=no -j15
        WORKING_DIRECTORY "${GODOT_ENGINE_SRC_DIR}"

        # Crée le dossier .godot_bin s'il n'existe pas
        COMMAND ${CMAKE_COMMAND} -E make_directory "${GODOT_BIN_DIR}"

        # Copie le résultat de SCons vers .godot_bin
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GODOT_ENGINE_SRC_DIR}/bin/${EDITOR_BIN_NAME}" "${GODOT_EDITOR_EXE}"

        COMMENT "Compilation de l'Éditeur Godot via SCons et copie dans .godot_bin..."
        USES_TERMINAL
)

# Compilation du Template Release (Pour l'export final)
add_custom_target(build_engine_template
        # Lance SCons dans le dossier godot-encrypted
        COMMAND python -m SCons platform=${SCONS_PLATFORM} target=template_release module_openssl_enabled=yes disable_path_overrides=no -j15
        WORKING_DIRECTORY "${GODOT_ENGINE_SRC_DIR}"

        # Copie le résultat de SCons vers .godot_bin
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GODOT_ENGINE_SRC_DIR}/bin/${TEMPLATE_BIN_NAME}" "${GODOT_TEMPLATE_EXE}"

        COMMENT "Compilation du Template Godot via SCons et copie dans .godot_bin..."
        USES_TERMINAL
)

# ==============================================================
# --- 2. COMPILATION DU CODE C++ (DLL) ---
# ==============================================================
add_subdirectory(externals)
add_subdirectory(src)

# ==============================================================
# --- 3. CIBLES DE DÉVELOPPEMENT ---
# ==============================================================

add_custom_target(editor
        COMMAND ${GODOT_EDITOR_EXE} -e --path "${GODOT_PROJECT_DIR}"
        # Dépend du moteur éditeur ET de ta DLL
        DEPENDS build_engine_editor GodotPP
        COMMENT "Lancement de l'Éditeur Custom..."
        USES_TERMINAL
)

add_custom_target(play
        COMMAND ${GODOT_EDITOR_EXE} --path "${GODOT_PROJECT_DIR}" --editor --headless --quit || ${CMAKE_COMMAND} -E echo "Importation ok."
        COMMAND ${GODOT_EDITOR_EXE} --path "${GODOT_PROJECT_DIR}"
        DEPENDS build_engine_editor GodotPP
        COMMENT "Lancement du jeu via l'Éditeur..."
        USES_TERMINAL
)

add_custom_target(cheat
        COMMAND ${GODOT_EDITOR_EXE} --path "${GODOT_CHEAT_DIR}" --editor --headless --quit || ${CMAKE_COMMAND} -E echo "Importation ok."
        COMMAND ${GODOT_EDITOR_EXE} --path "${GODOT_CHEAT_DIR}"
        DEPENDS build_engine_editor GodotPP_Cheat
        COMMENT "Lancement du jeu (Cheat)..."
        USES_TERMINAL
)

add_custom_target(server
        COMMAND $<TARGET_FILE:GodotPPServer>
        DEPENDS GodotPPServer
        WORKING_DIRECTORY "$<TARGET_FILE_DIR:GodotPPServer>"
        COMMENT "Lancement du Serveur C++ Standalone..."
        USES_TERMINAL
)

# ==============================================================
# --- 4. LA CIBLE MAGIQUE : PACKAGE ---
# ==============================================================

add_custom_target(package
        COMMAND ${CMAKE_COMMAND} -E make_directory "${EXPORT_DIR}/bin/windows"

        # On copie l'exécutable Template (qui est maintenant dans .godot_bin)
        COMMAND ${CMAKE_COMMAND} -E copy "${GODOT_TEMPLATE_EXE}" "${EXPORT_DIR}/GodotPP.exe"

        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:GodotPP>" "${EXPORT_DIR}/bin/windows/GodotPP.dll"

        # On exporte le .pck
        COMMAND ${GODOT_EDITOR_EXE} --headless --path "${GODOT_PROJECT_DIR}" --export-pack "Windows Desktop" "${EXPORT_DIR}/GodotPP.pck"

        # L'export exige que le Template ET l'Éditeur soient compilés
        DEPENDS build_engine_editor build_engine_template GodotPP
        COMMENT "Assemblage et Exportation du jeu terminés dans ${EXPORT_DIR} !"
)

# ==============================================================
# --- 5. TEST DE LA VERSION FINALE ---
# ==============================================================

add_custom_target(play_final
        COMMAND "${EXPORT_DIR}/GodotPP.exe" --verbose
        DEPENDS package
        WORKING_DIRECTORY "${EXPORT_DIR}"
        COMMENT "Lancement de la version finale (standalone)..."
        USES_TERMINAL
)
