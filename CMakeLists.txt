cmake_minimum_required(VERSION 3.28)
project(SpaceGame LANGUAGES CXX C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

link_directories(${CMAKE_SOURCE_DIR}/lib)

find_package(GLFW3 REQUIRED)
include_directories(${GLFW3_INCLUDE_DIR})

find_package(STB)

find_package(ReactPhysics3D REQUIRED)

find_package(OpenGL 4.1 REQUIRED)

find_package(Lua REQUIRED)
include_directories(${LUA_INCLUDE_DIR})

# find_package(Freetype REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIR})

add_library(GLAD "src/glad.c")
add_library(STB "include/stb_image.cpp")

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.1
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

include_directories(src)

include_directories(include)

add_executable(SpaceGame 
src/main.cpp
)
target_compile_features(SpaceGame PRIVATE cxx_std_20)
target_link_libraries(SpaceGame PRIVATE OpenGL::GL ReactPhysics3D::ReactPhysics3D ${LUA_LIBRARIES} Freetype::Freetype ${GLFW3_LIBRARY} GLAD STB)
