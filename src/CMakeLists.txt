# Collect source files
set(ENGINE_SOURCES
    core/time_manager.cpp
    core/transform.cpp
    core/types.cpp
    math/rectangle.cpp
    platform/logging.cpp
    platform/file_system.cpp
    platform/platform.cpp
    platform/window.cpp
    input/input_manager.cpp
    rendering/renderer.cpp
    rendering/texture.cpp
    rendering/texture_atlas.cpp
    rendering/camera.cpp
    rendering/shader.cpp
    rendering/quad_renderer.cpp
    scene/scene_manager.cpp
)

set(ENGINE_HEADERS
    core/time_manager.h
    core/transform.h
    core/types.h
    math/vector.h
    math/rectangle.h
    platform/logging.h
    platform/file_system.h
    platform/platform.h
    platform/window.h
    input/input_manager.h
    rendering/renderer.h
    rendering/texture.h
    rendering/texture_atlas.h
    rendering/camera.h
    rendering/shader.h
    rendering/quad_renderer.h
    scene/scene.h
    scene/scene_manager.h
)

# Add nlohmann/json for metadata parsing
include(FetchContent)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)

# Engine library (for testing)
add_library(EngineLib STATIC
    ${ENGINE_SOURCES}
    ${ENGINE_HEADERS}
)

target_include_directories(EngineLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/external
)

target_link_libraries(EngineLib PUBLIC
    bgfx
    bx
    bimg
    glfw
    glm
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    $<$<AND:$<NOT:$<BOOL:${APPLE}>>,$<BOOL:${UNIX}>>:dl>
)

# Main executable
add_executable(Engine
    main.cpp
)

target_link_libraries(Engine PRIVATE
    EngineLib
)

# Set output directory
set_target_properties(Engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Copy compiled shaders next to the executable after build
add_custom_command(TARGET Engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${SHADER_OUT_DIR}
        $<TARGET_FILE_DIR:Engine>/shaders
    COMMENT "Copying compiled shaders to executable directory..."
)

# Install
install(TARGETS Engine DESTINATION bin)