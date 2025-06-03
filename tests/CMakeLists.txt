# Find GTest
find_package(GTest REQUIRED)

# Test sources
set(TEST_SOURCES
    test_main.cpp
    test_scanner.cpp
    ../src/DocumentScanner.cpp
    ../src/utils/ImageProcessor.cpp
    ../src/utils/ContourDetector.cpp
    ../src/utils/PerspectiveTransform.cpp
    ../src/utils/ValidationUtils.cpp
)

# Create test executable
add_executable(test_document_scanner ${TEST_SOURCES})

# Link test libraries
target_link_libraries(test_document_scanner 
    ${OpenCV_LIBS}
    GTest::GTest
    GTest::Main
)

# Add test to CTest
add_test(NAME DocumentScannerTests COMMAND test_document_scanner)