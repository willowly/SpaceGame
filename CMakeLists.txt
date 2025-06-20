cmake_minimum_required(VERSION 3.28)
project(SpaceGame LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(OpenGL REQUIRED)

find_package(Lua REQUIRED)
include_directories(${LUA_INCLUDE_DIR})

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.1
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

include_directories(src)

add_executable(SpaceGame 
src/main.cpp
)
target_compile_features(SpaceGame PRIVATE cxx_std_20)
target_link_libraries(SpaceGame PRIVATE SFML::Graphics OpenGL::GL ${LUA_LIBRARIES})
