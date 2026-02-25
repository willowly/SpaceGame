cmake_minimum_required(VERSION 3.28)
project(SpaceGame LANGUAGES CXX C)

set(OUTPUT_DIR ${OUTPUT_DIR}/bin)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

if(WIN32)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /bigobj")
endif()

if(APPLE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive -fsanitize=address -Wall -Wextra -Werror=return-type -pedantic -Wno-unused-parameter")
endif()


if(APPLE)
    link_directories(${CMAKE_SOURCE_DIR}/lib/macos)
endif(APPLE)

find_library(COCOA_LIBRARY Cocoa)
find_library(IOKIT_LIBRARY IOKit)


if(APPLE)
set(Vulkan_LIBRARY     "/Users/willow/VulkanSDK/1.4.321.0/macOS/lib/libVulkan.dylib")
set(Vulkan_INCLUDE_DIR "/Users/willow/VulkanSDK/1.4.321.0/macOS/include")
set(VK_ICD_FILENAMES   "/Users/willow/VulkanSDK/1.4.321.0/macOS/share/vulkan/icd.d/MoltenVK_icd.json")
set(VK_LAYER_PATH,     "/Users/willow/VulkanSDK/1.4.321.0/macOS/share/vulkan/explicit_layer.d")
endif(APPLE)

if(WIN32)
set(Vulkan_LIBRARY     "C:/VulkanSDK/1.4.328.1/Lib/vulkan-1.lib")
set(Vulkan_INCLUDE_DIR "C:/VulkanSDK/1.4.328.1/Include")
endif(WIN32)
set(VK_LOADER_DEBUG,   all)

find_package(Vulkan REQUIRED)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_VULKAN_STATIC OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(GLFW_INCLUDE_NONE ON CACHE BOOL "" FORCE)

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw
    GIT_TAG        3.4
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)
message("Fetching glfw")
FetchContent_MakeAvailable(glfw)

find_package(OpenGL REQUIRED)

# find_package(Freetype REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIR})


add_library(STB "include/stb_image.cpp")
add_library(SIMPLEXNOISE "include/SimplexNoise.cpp")
# fucking glob wont work kms
add_library(Lua "include/lua/src/lapi.c" "include/lua/src/lauxlib.c" "include/lua/src/lbaselib.c" "include/lua/src/lcode.c" "include/lua/src/lcorolib.c" "include/lua/src/lctype.c" "include/lua/src/ldblib.c" "include/lua/src/ldebug.c" "include/lua/src/ldo.c" "include/lua/src/ldump.c" "include/lua/src/lfunc.c" "include/lua/src/lgc.c" "include/lua/src/linit.c" "include/lua/src/liolib.c" "include/lua/src/llex.c" "include/lua/src/lmathlib.c" "include/lua/src/lmem.c" "include/lua/src/loadlib.c" "include/lua/src/lobject.c" "include/lua/src/lopcodes.c" "include/lua/src/loslib.c" "include/lua/src/lparser.c" "include/lua/src/lstate.c" "include/lua/src/lstring.c" "include/lua/src/lstrlib.c" "include/lua/src/ltable.c" "include/lua/src/ltablib.c" "include/lua/src/ltm.c" "include/lua/src/lua.c" "include/lua/src/luac.c" "include/lua/src/lundump.c" "include/lua/src/lutf8lib.c" "include/lua/src/lvm.c" "include/lua/src/lzio.c")
include_directories("include/lua/src")

# option(TRACY_ENABLE "" ON)
# option(TRACY_ON_DEMAND "" ON)
# FetchContent_Declare( 
#     tracy
#     GIT_REPOSITORY https://github.com/wolfpld/tracy.git 
#     GIT_TAG master
#     GIT_SHALLOW TRUE
#     GIT_PROGRESS TRUE
# ) 
# FetchContent_MakeAvailable(tracy)


# Include Jolt
FetchContent_Declare(
        JoltPhysics
        GIT_REPOSITORY "https://github.com/jrouwe/JoltPhysics"
        GIT_TAG "v5.5.0"
		SOURCE_SUBDIR "Build"
)
FetchContent_MakeAvailable(JoltPhysics)


set_property(TARGET Jolt PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")

set_property(TARGET Jolt PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")


include_directories(src)

include_directories(include)

# include(tracy/public/tracy/tracy.hpp)

add_executable(SpaceGame 
src/main.cpp
include/volk.c
)


list(APPEND LINKED_LIBRARIES Lua glfw STB SIMPLEXNOISE Vulkan::Headers Jolt)
if(OSX)
    list(APPEND LINKED_LIBRARIES ${COCOA_LIBRARY} ${IOKIT_LIBRARY})
endif(OSX)




target_include_directories(SpaceGame PUBLIC ${JoltPhysics_SOURCE_DIR})

# important for windows

target_compile_features(SpaceGame PRIVATE cxx_std_20 )
target_link_libraries(SpaceGame PRIVATE ${LINKED_LIBRARIES})


add_custom_target(SHADERS)


if(WIN32)
    set(OUTPUT_DIR "${CMAKE_BINARY_DIR}")
endif(WIN32)
if(OSX)
    set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin")
endif(OSX)

add_custom_target(Assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${OUTPUT_DIR}
)
add_dependencies(SpaceGame Assets)

macro(compile_shader shader_path)
    add_custom_command(
        OUTPUT "${OUTPUT_DIR}/shaders/compiled/${shader_path}_frag.spv"
        COMMAND glslc "${CMAKE_CURRENT_LIST_DIR}/assets/shaders/${shader_path}.frag" -o "${OUTPUT_DIR}/shaders/compiled/${shader_path}_frag.spv"
        DEPENDS "${CMAKE_CURRENT_LIST_DIR}/assets/shaders/${shader_path}.frag"
    )
    list(APPEND SHADER_OUTPUTS "${OUTPUT_DIR}/shaders/compiled/${shader_path}_frag.spv")
    list(APPEND SHADER_SOURCES "${CMAKE_CURRENT_LIST_DIR}/assets/shaders/${shader_path}.frag")
    add_custom_command(
        OUTPUT "${OUTPUT_DIR}/shaders/compiled/${shader_path}_vert.spv"
        COMMAND glslc "${CMAKE_CURRENT_LIST_DIR}/assets/shaders/${shader_path}.vert" -o "${OUTPUT_DIR}/shaders/compiled/${shader_path}_vert.spv"
        DEPENDS "${CMAKE_CURRENT_LIST_DIR}/assets/shaders/${shader_path}.vert"
    )
    list(APPEND SHADER_OUTPUTS "${OUTPUT_DIR}/shaders/compiled/${shader_path}_vert.spv")
    list(APPEND SHADER_SOURCES "${CMAKE_CURRENT_LIST_DIR}/assets/shaders/${shader_path}.vert")
endmacro()


message("output dir: ${OUTPUT_DIR}")

compile_shader("lit")
compile_shader("terrain")
compile_shader("terrain-debug")
compile_shader("ui")
compile_shader("debug")
compile_shader("skybox")
compile_shader("construction")
compile_shader("particle")

compile_shader("shadow_test")

add_custom_target(SpaceGame_Shaders DEPENDS ${SHADER_OUTPUTS} SOURCES ${SHADER_SOURCES})
add_dependencies(SpaceGame_Shaders Assets)
add_dependencies(SpaceGame SpaceGame_Shaders)