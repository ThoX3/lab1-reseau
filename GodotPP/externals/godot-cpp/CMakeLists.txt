# Godot CPP Bindings
FetchContent_Declare(
        godot-cpp
        GIT_REPOSITORY https://github.com/godotengine/godot-cpp.git
        GIT_TAG godot-4.5-stable # Waiting for 4.6.2 official bindings
)
FetchContent_MakeAvailable(godot-cpp)
