# Automatic Document Scanner

**Computer Vision Final Project**  
**Course:** Computer Vision  
**Date:** May 2025

## 📋 Project Overview

This project implements an automatic document scanner system using C++ and OpenCV. The system takes a photograph of a physical document and produces a corrected, binarized, and flattened version similar to what a real scanner would produce.

### 🎯 Objectives

- **Input:** A photograph of a document (smartphone/webcam)
- **Output:** Top-down, perspective-corrected, black-and-white scanner-quality image
- **Performance:** Real-time processing with accuracy metrics

### ✅ Key Features

- Automatic document border detection
- Perspective distortion correction
- Image enhancement and binarization
- Batch processing capabilities
- Performance evaluation with ground truth validation

## 🏗️ Project Structure

```
document-scanner/
├── README.md
├── CMakeLists.txt
├── .gitignore
├── src/                    # Source code
│   ├── main.cpp           # Entry point and CLI interface
│   ├── DocumentScanner.h  # Main scanner class header
│   ├── DocumentScanner.cpp# Main scanner implementation
│   └── utils/             # Utility modules
│       ├── ImageProcessor.h/.cpp      # Image preprocessing
│       ├── ContourDetector.h/.cpp     # Edge detection & contours
│       ├── PerspectiveTransform.h/.cpp# Geometric transformations
│       └── ValidationUtils.h/.cpp     # Evaluation metrics
├── data/
│   ├── input/             # Test images (place your photos here)
│   ├── output/            # Processed results
│   └── ground_truth/      # Reference data for evaluation
│       └── coordinates.txt# Ground truth corner coordinates
├── tests/                 # Unit tests
├── docs/                  # Documentation
├── scripts/               # Build and utility scripts
│   ├── build.sh          # Build script for Linux/macOS
│   └── run_tests.sh      # Test runner script
└── build/                # Build directory (auto-generated)
```

## 🚀 Getting Started

### Prerequisites

**Required Dependencies:**

- **C++17** or higher
- **CMake 3.10+**
- **OpenCV 4.x** (core, imgproc, imgcodecs, highgui)
- **GTest** (for testing)

### Installation

#### Linux (Ubuntu/Debian)

```bash
# Install OpenCV and build tools
sudo apt-get update
sudo apt-get install build-essential cmake
sudo apt-get install libopencv-dev

# Install GTest for testing (optional)
sudo apt-get install libgtest-dev
```

#### macOS

```bash
# Using Homebrew
brew install cmake opencv

# Using MacPorts
sudo port install opencv4 +universal
```

#### Windows

```batch
# Use vcpkg for dependency management
git clone https://github.com/Microsoft/vcpkg.git
.\vcpkg\bootstrap-vcpkg.bat
.\vcpkg\vcpkg install opencv4[contrib]:x64-windows
```

### Building the Project

#### Quick Build (Linux/macOS)

```bash
# Clone the repository
git clone https://github.com/antoniooodev/document-scanner.git
cd document-scanner

# Build using the provided script
chmod +x scripts/build.sh
./scripts/build.sh
```

#### Manual Build

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake -DCMAKE_BUILD_TYPE=Release ..

# Compile
make -j4

# The executable will be created as: ./DocumentScanner
```

#### Windows (Visual Studio)

```batch
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=[vcpkg_path]/scripts/buildsystems/vcpkg.cmake ..
# Open DocumentScanner.sln in Visual Studio and build
```

## 📖 Usage

### Command Line Interface

#### Basic Usage

```bash
# Process a single image
./DocumentScanner input_image.jpg output_image.jpg

# Example with sample data
./DocumentScanner ../data/input/document.jpg ../data/output/scanned.jpg
```

#### Advanced Options

```bash
# Enable validation mode (requires ground truth data)
./DocumentScanner input.jpg output.jpg --validate --ground-truth ../data/ground_truth/coordinates.txt

# Batch processing
./DocumentScanner --batch ../data/input/ ../data/output/

# Verbose output with timing information
./DocumentScanner input.jpg output.jpg --verbose

# Help and usage information
./DocumentScanner --help
```

### Programming Interface

```cpp
#include "DocumentScanner.h"

int main() {
    // Initialize scanner
    DocumentScanner scanner;

    // Process single image
    double processingTime = scanner.processImage("input.jpg", "output.jpg");

    // Enable validation mode
    scanner.setValidationMode(true, "coordinates.txt");

    // Batch processing
    std::vector<double> times = scanner.batchProcess("input_dir/", "output_dir/");

    // Get detailed statistics
    auto stats = scanner.getLastStats();
    std::cout << "Total time: " << stats.totalTime << "ms" << std::endl;

    return 0;
}
```

## 🔧 Processing Pipeline

Our document scanner implements a **9-step pipeline**:

### 1. **Input Acquisition**

- Load source image in color using `cv::imread()`
- Support for common formats: JPG, PNG, BMP, TIFF

### 2. **Resize and Grayscale Conversion**

- Resize to maximum width of 1000px to standardize processing
- Convert to grayscale using `cv::cvtColor()`

### 3. **Gaussian Blur (Smoothing)**

- Apply 5×5 Gaussian kernel using `cv::GaussianBlur()`
- Reduces noise while preserving edges

### 4. **Canny Edge Detection**

- Extract prominent edges using `cv::Canny()`
- Dual threshold values: 50 (low), 150 (high)

### 5. **Contour Extraction**

- Find all contours in edge map using `cv::findContours()`
- Sort contours by area (descending order)
- Approximate contours to polygons using `cv::approxPolyDP()`

### 6. **Vertex Ordering**

- Identify the largest four-point polygon (document boundary)
- Order corners consistently: top-left → top-right → bottom-right → bottom-left

### 7. **Perspective Transformation**

- Define target rectangle (800×800px) representing unwrapped document
- Compute 4×4 perspective transform matrix using `cv::getPerspectiveTransform()`
- Warp original color image using `cv::warpPerspective()`

### 8. **Post-processing**

- Convert warped image to grayscale (if not already)
- Apply adaptive thresholding using `cv::adaptiveThreshold()`
- Optional: brightness/contrast adjustment or histogram equalization

### 9. **Output and Validation**

- Save final scanned image using `cv::imwrite()`
- Compute accuracy metrics (corner error, IoU)
- Log processing times for performance analysis

## 📊 Evaluation Metrics

The system provides comprehensive evaluation against ground truth data:

### 1. **Corner Detection Accuracy**

- **Metric:** Average Euclidean distance error between detected and ground truth corners
- **Formula:** `error = (1/4) * Σ||detected_corner_i - truth_corner_i||`
- **Goal:** Minimize pixel-level error

### 2. **IoU (Intersection over Union)**

- **Metric:** Overlap between detected document area and ground truth
- **Formula:** `IoU = Area(intersection) / Area(union)`
- **Range:** 0.0 (no overlap) to 1.0 (perfect match)

### 3. **Processing Time**

- **Metric:** End-to-end processing time per image
- **Breakdown:** Time for each pipeline stage
- **Goal:** Real-time performance (< 100ms per image)

### Ground Truth Format

The `coordinates.txt` file should contain corner coordinates in this format:

```
image1.jpg: (x1,y1) (x2,y2) (x3,y3) (x4,y4)
image2.jpg: (x1,y1) (x2,y2) (x3,y3) (x4,y4)
```

## 🧪 Testing

### Running Tests

```bash
# Build and run all tests
./scripts/run_tests.sh

# Manual test execution
cd build
ctest --verbose

# Run specific test
./tests/test_document_scanner
```

### Test Coverage

- **Unit Tests:** Each utility class (ImageProcessor, ContourDetector, etc.)
- **Integration Tests:** Complete pipeline with sample documents
- **Performance Tests:** Processing time benchmarks
- **Validation Tests:** Accuracy against ground truth data

## 🎯 Development Workflow

### Branch Strategy

- `main` - Stable release branch
- `develop` - Integration branch
- `feature/image-preprocessing` - Basic image operations
- `feature/edge-detection` - Canny and contour extraction
- `feature/perspective-correction` - Geometric transformations
- `feature/post-processing` - Binarization and enhancement
- `feature/evaluation` - Metrics and validation

### Contributing

1. Create feature branch from `develop`
2. Implement functionality with tests
3. Ensure all tests pass
4. Submit pull request with description
5. Code review and merge

### Code Style Guidelines

- Follow [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
- Use meaningful variable and function names
- Add comprehensive comments for complex algorithms
- Include error handling and validation
- Maintain consistent indentation (4 spaces)

## 🚀 Performance Optimization

### Memory Efficiency

- Use `cv::Mat` efficiently (avoid unnecessary copies)
- Implement RAII principles for resource management
- Profile memory usage with Valgrind

### Speed Optimization

- Multi-threading for batch processing
- Optimize critical loops
- Use OpenCV's optimized functions
- Consider GPU acceleration for large datasets

### Profiling Tools

- **Linux:** `gprof`, `perf`, `valgrind`
- **Windows:** Visual Studio Profiler
- **macOS:** Instruments

## 📈 Results and Benchmarks

### Expected Performance

- **Accuracy:** > 95% corner detection accuracy
- **IoU Score:** > 0.85 for well-lit documents
- **Speed:** < 50ms per image on modern hardware
- **Memory:** < 100MB peak usage for 1000px images

### Hardware Requirements

- **Minimum:** 2GB RAM, dual-core processor
- **Recommended:** 4GB+ RAM, quad-core processor
- **GPU:** Optional (for acceleration)

## ❗ Limitations and Assumptions

### Current Limitations

- Single document per image
- Requires reasonably well-lit conditions
- No OCR or content understanding
- Assumes document is not severely wrinkled
- Works best with rectangular documents

### Future Enhancements

- Multiple document detection
- Better handling of poor lighting
- Mobile app integration
- GPU acceleration
- Machine learning-based improvements

## 🆘 Troubleshooting

### Common Issues

#### Build Errors

```bash
# OpenCV not found
sudo apt-get install libopencv-dev

# CMake version too old
sudo apt-get install cmake
```

#### Runtime Issues

```bash
# Image loading fails
# Check file path and format support

# No contours detected
# Verify image quality and lighting
# Adjust Canny thresholds in configuration
```

#### Performance Issues

```bash
# Slow processing
# Check image resolution (resize if too large)
# Verify OpenCV is using optimized builds
```

## 📞 Contact and Support

- **Course:** Computer Vision Final Project
- **Institution:** [University of Padua]

### Getting Help

1. Check the [Issues](https://github.com/antoniooodev/document-scanner/issues) page
2. Review the documentation in `/docs`
3. Contact the development team
4. Refer to OpenCV documentation for technical details

## 📜 License

This project is developed for academic purposes as part of a Computer Vision course. Please respect academic integrity guidelines when using this code.

---

**Happy Scanning! 📄✨**
