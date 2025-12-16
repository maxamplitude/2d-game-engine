# Examples directory CMakeLists.txt

# Input and Camera Demo
add_executable(InputCameraDemo
    input_camera_demo.cpp
)

target_link_libraries(InputCameraDemo PRIVATE
    EngineLib
)

target_include_directories(InputCameraDemo PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Set output directory
set_target_properties(InputCameraDemo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Optional: Add other examples here as you create them