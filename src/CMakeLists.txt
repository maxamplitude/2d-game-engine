# Collect source files
set(ENGINE_SOURCES
    core/transform.cpp
    core/time_manager.cpp
    math/rectangle.cpp
    rendering/texture_atlas.cpp
    rendering/camera.cpp
    animation/animation_controller.cpp
    animation/animation_state_machine.cpp
    input/input_manager.cpp
)

set(ENGINE_HEADERS
    core/transform.h
    core/time_manager.h
    math/rectangle.h
    rendering/texture_atlas.h
    rendering/camera.h
    animation/animation_controller.h
    animation/animation_state_machine.h
    input/input_manager.h
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
)

target_link_libraries(EngineLib PUBLIC
    sfml-system
    sfml-window
    sfml-graphics
    sfml-audio
    nlohmann_json::nlohmann_json
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

# Install
install(TARGETS Engine DESTINATION bin)