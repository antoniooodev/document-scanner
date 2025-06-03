#ifndef CONTOUR_DETECTOR_H
#define CONTOUR_DETECTOR_H

#include <opencv2/opencv.hpp>
#include <vector>

/**
 * Handles edge detection and contour extraction
 */
class ContourDetector
{
public:
    ContourDetector() = default;

    /**
     * Complete contour detection pipeline
     * @param input Preprocessed grayscale image
     * @param cannyLow Lower threshold for Canny
     * @param cannyHigh Upper threshold for Canny
     * @return Four corner points of the largest rectangular contour
     */
    std::vector<cv::Point2f> detectDocumentContour(const cv::Mat &input,
                                                   double cannyLow = 50.0,
                                                   double cannyHigh = 150.0);

    /**
     * Apply Canny edge detection
     * @param input Blurred grayscale image
     * @param lowThreshold Lower threshold
     * @param highThreshold Upper threshold
     * @return Edge map
     */
    cv::Mat detectEdges(const cv::Mat &input, double lowThreshold, double highThreshold);

    /**
     * Find and filter contours
     * @param edgeMap Binary edge image
     * @return Vector of contours sorted by area (descending)
     */
    std::vector<std::vector<cv::Point>> findContours(const cv::Mat &edgeMap);

    /**
     * Approximate contour to polygon and validate as document
     * @param contour Input contour
     * @return Four corner points if valid document, empty vector otherwise
     */
    std::vector<cv::Point2f> approximateToRectangle(const std::vector<cv::Point> &contour);

private:
    /**
     * Check if contour represents a valid document shape
     * @param approx Approximated polygon points
     * @return True if valid document contour
     */
    bool isValidDocumentContour(const std::vector<cv::Point> &approx);

    /**
     * Calculate contour area
     */
    double calculateArea(const std::vector<cv::Point> &contour);
};