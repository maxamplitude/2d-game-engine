# Collect source files
set(ENGINE_SOURCES
    core/transform.cpp
    core/time_manager.cpp
    math/rectangle.cpp
)

set(ENGINE_HEADERS
    core/transform.h
    core/time_manager.h
    math/rectangle.h
)

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