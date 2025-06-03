#!/bin/bash

# Test runner script

set -e

if [ ! -d "build" ]; then
    echo "Build directory not found. Run build.sh first."
    exit 1
fi

cd build

echo "Running unit tests..."
ctest --verbose

echo "Running integration tests with sample data..."
if [ -f "../data/input/sample.jpg" ]; then
    ./DocumentScanner ../data/input/sample.jpg ../data/output/test_output.jpg --validate
else
    echo "No sample data found. Skipping integration tests."
fi

echo "All tests completed!"