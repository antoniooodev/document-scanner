# Document Scanner

An optimized C++ document scanner using OpenCV that automatically detects document boundaries in images and provides perspective correction.
The final results for performance metrics are saved in complete_results.txt in the output folder.

## Project Structure

```
DocumentScanner/
├── CMakeLists.txt          # Main build configuration
├── include/               # Header files
│   ├── geometry_utils.h
│   ├── image_preprocessing.h
│   ├── contour_analysis.h
│   ├── document_detector.h
│   ├── file_io.h
│   ├── evaluation.h
│   └── visualization.h
├── src/                   # Source files
│   ├── main.cpp
│   ├── geometry_utils.cpp
│   ├── image_preprocessing.cpp
│   ├── contour_analysis.cpp
│   ├── document_detector.cpp
│   ├── file_io.cpp
│   ├── evaluation.cpp
│   └── visualization.cpp
└── data/                  # Test data
    ├── input/
    ├── output/
    └── ground_truth/
```

## Building

### Prerequisites

- OpenCV 4.x
- CMake 3.16+
- C++17 compatible compiler

### Build Instructions

1. **Quick build:**

   ```bash
   mkdir build
   cd build/
   cmake ..
   make

## Usage

### Single Image Processing

```bash
./DocumentScanner input_image.jpg ground_truth.txt
```

### Dataset Processing

```bash
./DocumentScanner --dataset /path/to/dataset/
```

Expected dataset structure:

```
dataset/
├── img_1.png
├── img_2.png
...
└── img_10.png
```

## Features

- **Optimized Parameters**: Tuned for IoU performance (0.84+ average)
- **Multiple Detection Methods**:
  - Contour approximation
  - Minimum area rectangles
  - Optional line segment detection
- **Quality Scoring**: Based on area, aspect ratio, edge strength, and document whiteness
- **Visualization**: Outputs comparison images with detected vs ground truth boxes

## Performance Metrics

- **Corner Detection Accuracy**: Distance error between detected and ground truth corners
- **IoU (Intersection over Union)**: Overlap between detected and ground truth document areas
- **Processing Time**: Optimized for real-time performance

## Algorithm Overview

1. **Preprocessing**: CLAHE enhancement, gradient calculation, morphological operations
2. **Contour Detection**: Multiple algorithms for robust detection
3. **Quality Scoring**: Multi-factor evaluation of candidate quadrilaterals
4. **Refinement**: Border-aware coordinate adjustment
5. **Visualization**: Result comparison and export
