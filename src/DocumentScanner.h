#ifndef DOCUMENT_SCANNER_H
#define DOCUMENT_SCANNER_H

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <chrono>

#include "utils/ImageProcessor.h"
#include "utils/ContourDetector.h"
#include "utils/PerspectiveTransform.h"
#include "utils/ValidationUtils.h"

/**
 * Main class for automatic document scanning
 * Implements the complete pipeline from input image to scanner-quality output
 */
class DocumentScanner
{
public:
    /**
     * Constructor with default parameters
     */
    DocumentScanner();

    /**
     * Process a single image from file path
     * @param inputPath Path to input image
     * @param outputPath Path for output image
     * @return Processing time in milliseconds
     */
    double processImage(const std::string &inputPath, const std::string &outputPath);

    /**
     * Process image from cv::Mat
     * @param input Input image matrix
     * @return Processed scanner output
     */
    cv::Mat processImage(const cv::Mat &input);

    /**
     * Batch process multiple images
     * @param inputDir Input directory path
     * @param outputDir Output directory path
     * @return Vector of processing times
     */
    std::vector<double> batchProcess(const std::string &inputDir, const std::string &outputDir);

    /**
     * Enable/disable validation mode
     * @param enable Enable validation
     * @param groundTruthFile Path to coordinates.txt file
     */
    void setValidationMode(bool enable, const std::string &groundTruthFile = "");

    /**
     * Get last processing statistics
     */
    struct ProcessingStats
    {
        double totalTime;
        double preprocessingTime;
        double edgeDetectionTime;
        double contourExtractionTime;
        double perspectiveTransformTime;
        double postProcessingTime;
        bool success;
        std::string errorMessage;
    };

    ProcessingStats getLastStats() const { return lastStats; }

private:
    // Processing modules
    ImageProcessor processor;
    ContourDetector detector;
    PerspectiveTransform transformer;
    ValidationUtils validator;

    // Configuration parameters
    struct Config
    {
        int maxWidth = 1000;
        cv::Size targetSize = cv::Size(800, 800);
        int gaussianKernel = 5;
        double cannyLow = 50.0;
        double cannyHigh = 150.0;
        bool enableValidation = false;
        std::string groundTruthFile = "";
    } config;

    // Statistics
    ProcessingStats lastStats;

    // Helper methods
    void updateProcessingTime(const std::string &stage, double time);
    bool validateInput(const cv::Mat &input);
};