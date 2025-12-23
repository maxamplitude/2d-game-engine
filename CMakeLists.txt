cmake_minimum_required(VERSION 3.20)
project(GameEngine VERSION 0.1.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4 /WX-)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# Build options
option(BUILD_TESTS "Build unit tests" ON)
# SFML-based examples are deprecated/disabled while migrating off SFML.
option(BUILD_EXAMPLES "Build example games" OFF)

include(FetchContent)

# --- BGFX + deps ---
FetchContent_Declare(
    bgfx.cmake
    GIT_REPOSITORY https://github.com/bkaradzic/bgfx.cmake.git
    GIT_TAG v1.128.8786-481
)
set(BGFX_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BGFX_BUILD_TOOLS ON CACHE BOOL "" FORCE) # needed for shaderc
set(BGFX_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(bgfx.cmake)

# --- GLFW ---
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# --- GLM ---
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.8
)
FetchContent_MakeAvailable(glm)

# --- spdlog ---
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.12.0
)
FetchContent_MakeAvailable(spdlog)

# --- STB single headers ---
file(DOWNLOAD
    https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
    ${CMAKE_SOURCE_DIR}/external/stb_image.h
)
file(DOWNLOAD
    https://raw.githubusercontent.com/nothings/stb/master/stb_truetype.h
    ${CMAKE_SOURCE_DIR}/external/stb_truetype.h
)

# --- Shader compilation paths (shared with subdirs) ---
# Note: shaderc is produced under cmake/bgfx in the bgfx.cmake build directory.
set(SHADERC_EXE "${CMAKE_BINARY_DIR}/_deps/bgfx.cmake-build/cmake/bgfx/shaderc${CMAKE_EXECUTABLE_SUFFIX}")
set(SHADERC_INCLUDE_DIR "${CMAKE_BINARY_DIR}/_deps/bgfx.cmake-src/bgfx/src")
set(SHADER_SRC_DIR "${CMAKE_SOURCE_DIR}/assets/shaders")
set(SHADER_OUT_DIR "${CMAKE_BINARY_DIR}/shaders")
file(MAKE_DIRECTORY ${SHADER_OUT_DIR})

if(WIN32)
    set(SHADERC_PLATFORM "windows")
elseif(APPLE)
    set(SHADERC_PLATFORM "osx")
else()
    set(SHADERC_PLATFORM "linux")
endif()

# Add subdirectories
add_subdirectory(src)

add_custom_target(compile_shaders ALL
    # GL
    COMMAND ${SHADERC_EXE}
        -f ${SHADER_SRC_DIR}/sprite.vert.sc
        -o ${SHADER_OUT_DIR}/sprite.vert.gl.bin
        --platform ${SHADERC_PLATFORM}
        --type vertex
        -p 120
        -i ${SHADERC_INCLUDE_DIR}
        --varyingdef ${SHADER_SRC_DIR}/varying.def.sc

    COMMAND ${SHADERC_EXE}
        -f ${SHADER_SRC_DIR}/sprite.frag.sc
        -o ${SHADER_OUT_DIR}/sprite.frag.gl.bin
        --platform ${SHADERC_PLATFORM}
        --type fragment
        -p 120
        -i ${SHADERC_INCLUDE_DIR}
        --varyingdef ${SHADER_SRC_DIR}/varying.def.sc

    # Vulkan (SPIR-V)
    COMMAND ${SHADERC_EXE}
        -f ${SHADER_SRC_DIR}/sprite.vert.sc
        -o ${SHADER_OUT_DIR}/sprite.vert.vk.bin
        --platform ${SHADERC_PLATFORM}
        --type vertex
        -p spirv
        -i ${SHADERC_INCLUDE_DIR}
        --varyingdef ${SHADER_SRC_DIR}/varying.def.sc

    COMMAND ${SHADERC_EXE}
        -f ${SHADER_SRC_DIR}/sprite.frag.sc
        -o ${SHADER_OUT_DIR}/sprite.frag.vk.bin
        --platform ${SHADERC_PLATFORM}
        --type fragment
        -p spirv
        -i ${SHADERC_INCLUDE_DIR}
        --varyingdef ${SHADER_SRC_DIR}/varying.def.sc

    DEPENDS shaderc
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Compiling BGFX shaders (GL/VK)..."
    VERBATIM
)

# Only build Metal shaders on macOS hosts
if(APPLE)
    add_custom_command(TARGET compile_shaders POST_BUILD
        COMMAND ${SHADERC_EXE}
            -f ${SHADER_SRC_DIR}/sprite.vert.sc
            -o ${SHADER_OUT_DIR}/sprite.vert.mtl.bin
            --platform osx
            --type vertex
            -p metal
            -i ${SHADERC_INCLUDE_DIR}
            --varyingdef ${SHADER_SRC_DIR}/varying.def.sc

        COMMAND ${SHADERC_EXE}
            -f ${SHADER_SRC_DIR}/sprite.frag.sc
            -o ${SHADER_OUT_DIR}/sprite.frag.mtl.bin
            --platform osx
            --type fragment
            -p metal
            -i ${SHADERC_INCLUDE_DIR}
            --varyingdef ${SHADER_SRC_DIR}/varying.def.sc

        COMMENT "Compiling BGFX shaders (Metal)..."
        VERBATIM
    )
endif()

# Only build DirectX shaders on Windows hosts
if(WIN32)
    add_custom_command(TARGET compile_shaders POST_BUILD
        COMMAND ${SHADERC_EXE}
            -f ${SHADER_SRC_DIR}/sprite.vert.sc
            -o ${SHADER_OUT_DIR}/sprite.vert.dx11.bin
            --platform windows
            --type vertex
            -p vs_5_0
            -i ${SHADERC_INCLUDE_DIR}
            --varyingdef ${SHADER_SRC_DIR}/varying.def.sc

        COMMAND ${SHADERC_EXE}
            -f ${SHADER_SRC_DIR}/sprite.frag.sc
            -o ${SHADER_OUT_DIR}/sprite.frag.dx11.bin
            --platform windows
            --type fragment
            -p ps_5_0
            -i ${SHADERC_INCLUDE_DIR}
            --varyingdef ${SHADER_SRC_DIR}/varying.def.sc

        COMMENT "Compiling BGFX shaders (DX11)..."
        VERBATIM
    )
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

