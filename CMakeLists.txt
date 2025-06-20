cmake_minimum_required(VERSION 3.16)
project(DocumentScanner VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV
find_package(OpenCV REQUIRED)

# Include directories
include_directories(include)
include_directories(${OpenCV_INCLUDE_DIRS})

# Source files
set(SOURCES
    src/main.cpp
    src/geometry_utils.cpp
    src/document_transform.cpp
    src/image_preprocessing.cpp
    src/contour_analysis.cpp
    src/document_detector.cpp
    src/file_io.cpp
    src/evaluation.cpp
    src/visualization.cpp
)

# Create executable
add_executable(DocumentScanner ${SOURCES})

# Link libraries
target_link_libraries(DocumentScanner ${OpenCV_LIBS})