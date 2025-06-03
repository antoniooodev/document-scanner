#ifndef VALIDATION_UTILS_H
#define VALIDATION_UTILS_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

/**
 * Validation and evaluation utilities
 */
class ValidationUtils
{
public:
    ValidationUtils() = default;

    /**
     * Load ground truth coordinates from file
     * @param filename Path to coordinates.txt
     * @return Vector of ground truth corner points
     */
    std::vector<cv::Point2f> loadGroundTruth(const std::string &filename);

    /**
     * Calculate corner detection accuracy (average distance error)
     * @param detected Detected corner points
     * @param groundTruth Ground truth corner points
     * @return Average Euclidean distance error
     */
    double calculateCornerAccuracy(const std::vector<cv::Point2f> &detected,
                                   const std::vector<cv::Point2f> &groundTruth);

    /**
     * Calculate IoU between detected and ground truth areas
     * @param detected Detected corner points
     * @param groundTruth Ground truth corner points
     * @param imageSize Size of the image for area calculation
     * @return Intersection over Union score (0-1)
     */
    double calculateIoU(const std::vector<cv::Point2f> &detected,
                        const std::vector<cv::Point2f> &groundTruth,
                        const cv::Size &imageSize);

    /**
     * Generate evaluation report
     * @param cornerError Corner detection error
     * @param iouScore IoU score
     * @param processingTime Processing time in ms
     * @return Formatted report string
     */
    std::string generateReport(double cornerError, double iouScore, double processingTime);

private:
    /**
     * Calculate polygon area from points
     */
    double calculatePolygonArea(const std::vector<cv::Point2f> &points);

    /**
     * Check if polygon is valid (no self-intersections)
     */
    bool isValidPolygon(const std::vector<cv::Point2f> &points);
};

#endif // Headers