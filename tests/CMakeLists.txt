# Download Catch2
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(Catch2)

# Collect all test files
file(GLOB_RECURSE TEST_SOURCES
    "*.cpp"
)

# Test executable
add_executable(EngineTests
    ${TEST_SOURCES}
)

target_link_libraries(EngineTests PRIVATE
    EngineLib
    Catch2::Catch2WithMain
)

target_include_directories(EngineTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

# Copy test assets to build directory
file(COPY ${CMAKE_SOURCE_DIR}/assets/test_data
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Register with CTest
include(CTest)
include(Catch)
catch_discover_tests(EngineTests)

# Set output directory
set_target_properties(EngineTests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)