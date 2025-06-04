cmake_minimum_required(VERSION 3.10)
project(DocumentScanner VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV
find_package(OpenCV REQUIRED)
if(OpenCV_FOUND)
    message(STATUS "OpenCV version: ${OpenCV_VERSION}")
    message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")
    message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")
else()
    message(FATAL_ERROR "OpenCV not found!")
endif()

# Include directories
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(src)
include_directories(src/utils)

# Compiler flags
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3")
elseif(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /O2")
endif()

# Debug/Release configurations
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Source files
set(SOURCES
    src/main.cpp
    src/DocumentScanner.cpp
    src/utils/ImageProcessor.cpp
    src/utils/ContourDetector.cpp
    src/utils/PerspectiveTransform.cpp
    src/utils/ValidationUtils.cpp
)

# Header files (for IDE support)
set(HEADERS
    src/DocumentScanner.h
    src/utils/ImageProcessor.h
    src/utils/ContourDetector.h
    src/utils/PerspectiveTransform.h
    src/utils/ValidationUtils.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

# Enable testing
enable_testing()

# Find GTest (optional)
find_package(GTest QUIET)
if(GTest_FOUND)
    message(STATUS "GTest found, building tests")
    add_subdirectory(tests)
else()
    message(STATUS "GTest not found, skipping tests")
endif()

# Install targets
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# Copy data directory to build for easy testing
file(COPY ${CMAKE_SOURCE_DIR}/data DESTINATION ${CMAKE_BINARY_DIR})

# Print build information
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")