# Find required packages
find_package(OpenCV REQUIRED)

# Include directories from parent
include_directories(../include)
include_directories(${OpenCV_INCLUDE_DIRS})

# Test sources (excluding main.cpp)
set(TEST_SOURCES
    ../src/geometry_utils.cpp
    ../src/image_preprocessing.cpp
    ../src/contour_analysis.cpp
    ../src/document_detector.cpp
    ../src/file_io.cpp
    ../src/evaluation.cpp
    ../src/visualization.cpp
)

# Create test executable
add_executable(test_document_scanner 
    test_document_scanner.cpp
    ${TEST_SOURCES}
)

# Link libraries
target_link_libraries(test_document_scanner ${OpenCV_LIBS})

# Add test
add_test(NAME DocumentScannerTests COMMAND test_document_scanner)
