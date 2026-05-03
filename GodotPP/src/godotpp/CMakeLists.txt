# ==============================================================================
# 1. LE CLIENT NORMAL (Sain)
# ==============================================================================
add_library(GodotPP SHARED
        src/gd_example.cpp
        src/register_types.cpp
)

target_include_directories(GodotPP PRIVATE include)

target_link_libraries(GodotPP PRIVATE
        godot::cpp
        snl
        mon_openssl_wrapper
)

add_custom_command(TARGET GodotPP POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${GODOT_OUTPUT_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:GodotPP>" "${GODOT_OUTPUT_DIR}/$<TARGET_FILE_NAME:GodotPP>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/godotpp.gdextension.in" "${GODOT_PROJECT_DIR}/godotpp.gdextension"
    COMMENT "Installing NORMAL GDExtension to ${GODOT_OUTPUT_DIR}..."
)

# ==============================================================================
# 2. LE CLIENT CHEAT (Hacker)
# ==============================================================================
add_library(GodotPP_Cheat SHARED
    src/gd_example_cheat.cpp
    src/register_types.cpp
)

target_include_directories(GodotPP_Cheat PRIVATE include)

target_link_libraries(GodotPP_Cheat PRIVATE
    godot::cpp
    snl
    mon_openssl_wrapper
)

add_custom_command(TARGET GodotPP_Cheat POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${GODOT_PROJECT_DIR}" "${GODOT_CHEAT_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${GODOT_CHEAT_OUTPUT_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:GodotPP_Cheat>" "${GODOT_CHEAT_OUTPUT_DIR}/GodotPP.dll"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/godotpp.gdextension.in" "${GODOT_CHEAT_DIR}/godotpp.gdextension"
    COMMENT "Clonage du jeu et installation du CHEAT dans ${GODOT_CHEAT_DIR}..."
)
