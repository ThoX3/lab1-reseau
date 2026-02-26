cmake_minimum_required(VERSION 3.15)
project(SNL LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Rust_TOOLCHAIN "stable")

if(NOT TARGET Corrosion::Generator)
    include(FetchContent)
    FetchContent_Declare(
        Corrosion
        GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
        GIT_TAG v0.6.1
    )
    FetchContent_MakeAvailable(Corrosion)
endif()

corrosion_import_crate(MANIFEST_PATH Cargo.toml)
target_include_directories(snl INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/snl/include)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    add_subdirectory(samples/cpp)
endif()