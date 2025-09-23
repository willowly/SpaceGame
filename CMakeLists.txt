cmake_minimum_required(VERSION 3.28)
project(SpaceGame LANGUAGES CXX C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive -Wall -Wextra -pedantic -fsanitize=address,undefined -Wno-unused-parameter")

if(WIN32)
    add_library(GLF3 "lib/win/glfw-win.lib")
endif(WIN32)

if(APPLE)
    link_directories(${CMAKE_SOURCE_DIR}/lib/macos)
endif(APPLE)

find_library(COCOA_LIBRARY Cocoa)
find_library(IOKIT_LIBRARY IOKit)



set(Vulkan_LIBRARY     "/Users/willow/VulkanSDK/1.4.321.0/macOS/lib/libVulkan.dylib")
set(Vulkan_INCLUDE_DIR "/Users/willow/VulkanSDK/1.4.321.0/macOS/include")
set(VK_ICD_FILENAMES   "/Users/willow/VulkanSDK/1.4.321.0/macOS/share/vulkan/icd.d/MoltenVK_icd.json")
set(VK_LAYER_PATH,     "/Users/willow/VulkanSDK/1.4.321.0/macOS/share/vulkan/explicit_layer.d")
set(VK_LOADER_DEBUG,   all)

find_package(Vulkan REQUIRED)
find_package(GLFW3 REQUIRED)
include_directories(${GLFW3_INCLUDE_DIR})

message(${GLFW3_LIBRARY})

find_package(ReactPhysics3D REQUIRED)

find_package(OpenGL REQUIRED)

find_package(Lua REQUIRED)
include_directories(${LUA_INCLUDE_DIR})

# find_package(Freetype REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIR})

add_library(GLAD "src/glad.c")
add_library(STB "include/stb_image.cpp")
add_library(SIMPLEXNOISE "include/SimplexNoise.cpp")

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
target_compile_features(SpaceGame PRIVATE cxx_std_20 )
target_link_libraries(SpaceGame PRIVATE OpenGL::GL ReactPhysics3D::ReactPhysics3D ${LUA_LIBRARIES} Freetype::Freetype ${GLFW3_LIBRARY} GLAD STB ${COCOA_LIBRARY} ${IOKIT_LIBRARY} SIMPLEXNOISE Vulkan::Vulkan)

macro(compile_shader shader_path)
    add_custom_command(
        TARGET SpaceGame
        COMMAND glslc "${CMAKE_SOURCE_DIR}/build/bin/shaders/${shader_path}.frag" -o "${CMAKE_SOURCE_DIR}/build/bin/shaders/compiled/${shader_path}_frag.spv"
    )
    add_custom_command(
        TARGET SpaceGame
        COMMAND glslc "${CMAKE_SOURCE_DIR}/build/bin/shaders/${shader_path}.vert" -o "${CMAKE_SOURCE_DIR}/build/bin/shaders/compiled/${shader_path}_vert.spv"
    )
endmacro()


compile_shader("shader")