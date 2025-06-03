#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H

#include <opencv2/opencv.hpp>

/**
 * Handles initial image preprocessing steps
 */
class ImageProcessor
{
public:
    ImageProcessor() = default;

    /**
     * Complete preprocessing pipeline
     * @param input Source image
     * @param maxWidth Maximum width for resizing
     * @param kernelSize Gaussian blur kernel size
     * @return Preprocessed grayscale image
     */
    cv::Mat preprocess(const cv::Mat &input, int maxWidth = 1000, int kernelSize = 5);

    /**
     * Resize image maintaining aspect ratio
     * @param input Input image
     * @param maxWidth Maximum width
     * @return Resized image
     */
    cv::Mat resizeImage(const cv::Mat &input, int maxWidth);

    /**
     * Convert to grayscale
     * @param input Color input image
     * @return Grayscale image
     */
    cv::Mat convertToGrayscale(const cv::Mat &input);

    /**
     * Apply Gaussian blur for noise reduction
     * @param input Input image
     * @param kernelSize Kernel size (should be odd)
     * @return Blurred image
     */
    cv::Mat applyGaussianBlur(const cv::Mat &input, int kernelSize = 5);

private:
    double scaleFactor = 1.0; // Track scaling for coordinate conversion

public:
    double getScaleFactor() const { return scaleFactor; }
};