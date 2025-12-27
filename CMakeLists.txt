cmake_minimum_required(VERSION 3.24)
project(anim VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# =============================================================================
# Options
# =============================================================================
option(VKENGINE_ENABLE_VALIDATION "Enable Vulkan validation layers" ON)

# =============================================================================
# Find Vulkan
# =============================================================================
find_package(Vulkan REQUIRED)

# =============================================================================
# FetchContent Dependencies
# =============================================================================
include(FetchContent)

# SDL3
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.2.0
    GIT_SHALLOW TRUE
)

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_SHALLOW TRUE
)

# Vulkan Memory Allocator
FetchContent_Declare(
    VulkanMemoryAllocator
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG v3.3.0
    GIT_SHALLOW TRUE
)

# tinygltf (header-only)
FetchContent_Declare(
    tinygltf
    GIT_REPOSITORY https://github.com/syoyo/tinygltf.git
    GIT_TAG v2.9.3
    GIT_SHALLOW TRUE
)

message(STATUS "Fetching dependencies...")
FetchContent_MakeAvailable(SDL3 glm VulkanMemoryAllocator tinygltf)

# =============================================================================
# Source Files
# =============================================================================
set(ANIM_SOURCES
    src/main.cpp
    src/core/Window.cpp
    src/vulkan/Instance.cpp
    src/vulkan/Device.cpp
    src/vulkan/Swapchain.cpp
    src/vulkan/Sync.cpp
    src/vulkan/CommandPool.cpp
    src/vulkan/CommandBuffer.cpp
    src/vulkan/RenderPass.cpp
    src/vulkan/Framebuffer.cpp
    src/vulkan/Buffer.cpp
    src/vulkan/Pipeline.cpp
    src/renderer/Renderer.cpp
)

# =============================================================================
# Executable
# =============================================================================
add_executable(anim ${ANIM_SOURCES})

target_include_directories(anim PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${tinygltf_SOURCE_DIR}
)

target_link_libraries(anim PRIVATE
    Vulkan::Vulkan
    SDL3::SDL3
    glm::glm
    GPUOpen::VulkanMemoryAllocator
)

target_compile_definitions(anim PRIVATE
    GLM_FORCE_RADIANS
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    $<$<BOOL:${VKENGINE_ENABLE_VALIDATION}>:VKENGINE_ENABLE_VALIDATION>
)

# Platform-specific settings
if(APPLE)
    target_compile_definitions(anim PRIVATE VK_USE_PLATFORM_METAL_EXT)
elseif(UNIX)
    target_compile_definitions(anim PRIVATE VK_USE_PLATFORM_XLIB_KHR)
endif()

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(anim PRIVATE
        -Wall -Wextra -Wpedantic
        -Wno-missing-field-initializers
    )
endif()

# =============================================================================
# Shader compilation
# =============================================================================
find_program(GLSLC glslc)

if(GLSLC)
    file(GLOB SHADER_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag
    )

    foreach(SHADER ${SHADER_SOURCES})
        get_filename_component(SHADER_NAME ${SHADER} NAME)
        set(SHADER_OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${SHADER_NAME}.spv)

        add_custom_command(
            OUTPUT ${SHADER_OUTPUT}
            COMMAND ${GLSLC} ${SHADER} -o ${SHADER_OUTPUT}
            DEPENDS ${SHADER}
            COMMENT "Compiling shader: ${SHADER_NAME}"
        )
        list(APPEND SHADER_OUTPUTS ${SHADER_OUTPUT})
    endforeach()

    add_custom_target(shaders DEPENDS ${SHADER_OUTPUTS})
    add_dependencies(anim shaders)
else()
    message(WARNING "glslc not found. Shaders will not be compiled automatically.")
endif()

# =============================================================================
# Assets directory
# =============================================================================
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets/models)
