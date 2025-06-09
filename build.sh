#!/bin/bash

# Build script for Unix-like systems

set -e  # Exit on any error

echo "Building Document Scanner..."

# Create build directory
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build project
echo "Compiling..."
# Use different commands based on OS
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    make -j$(sysctl -n hw.ncpu)
else
    # Linux
    make -j$(nproc)
fi

echo "Build complete! Executable: ./DocumentScanner"

# Run tests if available
if [ -f "tests/test_document_scanner" ]; then
    echo "Running tests..."
    cd tests
    ./test_document_scanner
    cd ..
fi

echo "Done!"
