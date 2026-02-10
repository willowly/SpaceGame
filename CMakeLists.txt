cmake_minimum_required(VERSION 3.28)
project(SpaceGame LANGUAGES CXX C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive -fsanitize=address -Wall -Wextra -Werror=return-type -pedantic -Wno-unused-parameter")

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

find_package(ReactPhysics3D REQUIRED)

find_package(OpenGL REQUIRED)

find_package(Lua REQUIRED)
include_directories(${LUA_INCLUDE_DIR})

# find_package(Freetype REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIR})


add_library(STB "include/stb_image.cpp")
add_library(SIMPLEXNOISE "include/SimplexNoise.cpp")


option(TRACY_ENABLE "" ON)
option(TRACY_ON_DEMAND "" ON)
FetchContent_Declare( 
    tracy
    GIT_REPOSITORY https://github.com/wolfpld/tracy.git 
    GIT_TAG "v0.13.1"
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
) 
FetchContent_MakeAvailable(tracy)


# Include Jolt
FetchContent_Declare(
        JoltPhysics
        GIT_REPOSITORY "https://github.com/jrouwe/JoltPhysics"
        GIT_TAG "v5.5.0"
		SOURCE_SUBDIR "Build"
)
FetchContent_MakeAvailable(JoltPhysics)

target_compile_options(Jolt PUBLIC "-frtti")

# include(FetchContent)
# FetchContent_Declare(SFML
#     GIT_REPOSITORY https://github.com/SFML/SFML.git
#     GIT_TAG 3.0.1
#     GIT_SHALLOW ON
#     EXCLUDE_FROM_ALL
#     SYSTEM)
# FetchContent_MakeAvailable(SFML)

# add_library(IMGUI 
# "include/imgui/imgui.cpp"

# "include/imgui/imgui_demo.cpp"
# "include/imgui/imgui_draw.cpp"
# "include/imgui/imgui_widgets.cpp"
# "include/imgui/imgui_tables.cpp"

# "include/imgui/backends/imgui_impl_vulkan.cpp"
# "include/imgui/backends/imgui_impl_glfw.cpp"
# )

# target_compile_features(IMGUI PRIVATE cxx_std_20)
# target_include_directories(IMGUI PRIVATE include/imgui)


include_directories(src)

include_directories(include)

# include(tracy/public/tracy/tracy.hpp)

add_executable(SpaceGame 
src/main.cpp
include/volk.c
)


target_include_directories(SpaceGame PUBLIC ${JoltPhysics_SOURCE_DIR})

target_compile_features(SpaceGame PRIVATE cxx_std_20 )
target_link_libraries(SpaceGame PRIVATE ${LUA_LIBRARIES} glfw STB ${COCOA_LIBRARY} ${IOKIT_LIBRARY} SIMPLEXNOISE TracyClient Vulkan::Headers Jolt)


add_custom_target(SHADERS)

macro(compile_shader shader_path)
    add_custom_command(
        OUTPUT "${CMAKE_SOURCE_DIR}/build/bin/shaders/compiled/${shader_path}_frag.spv"
        COMMAND glslc "${CMAKE_SOURCE_DIR}/assets/shaders/${shader_path}.frag" -o "${CMAKE_SOURCE_DIR}/build/bin/shaders/compiled/${shader_path}_frag.spv"
    )
    list(APPEND SHADER_OUTPUTS "${CMAKE_SOURCE_DIR}/build/bin/shaders/compiled/${shader_path}_frag.spv")
    add_custom_command(
        OUTPUT "${CMAKE_SOURCE_DIR}/build/bin/shaders/compiled/${shader_path}_vert.spv"
        COMMAND glslc "${CMAKE_SOURCE_DIR}/assets/shaders/${shader_path}.vert" -o "${CMAKE_SOURCE_DIR}/build/bin/shaders/compiled/${shader_path}_vert.spv"
    )
    list(APPEND SHADER_OUTPUTS "${CMAKE_SOURCE_DIR}/build/bin/shaders/compiled/${shader_path}_vert.spv")
endmacro()

add_custom_target(Assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/bin
)
add_dependencies(SpaceGame Assets)

compile_shader("lit")
compile_shader("terrain")
compile_shader("terrain-debug")
compile_shader("ui")
compile_shader("debug")
compile_shader("skybox")
compile_shader("construction")

compile_shader("test")

add_custom_target(SpaceGame_Shaders DEPENDS ${SHADER_OUTPUTS})
add_dependencies(SpaceGame SpaceGame_Shaders)