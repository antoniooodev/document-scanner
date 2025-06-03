#ifndef PERSPECTIVE_TRANSFORM_H
#define PERSPECTIVE_TRANSFORM_H

#include <opencv2/opencv.hpp>
#include <vector>

/**
 * Handles vertex ordering and perspective transformation
 */
class PerspectiveTransform
{
public:
    PerspectiveTransform() = default;

    /**
     * Complete perspective correction pipeline
     * @param input Original color image
     * @param corners Four corner points (unordered)
     * @param targetSize Output rectangle size
     * @return Perspective-corrected image
     */
    cv::Mat correctPerspective(const cv::Mat &input,
                               const std::vector<cv::Point2f> &corners,
                               const cv::Size &targetSize = cv::Size(800, 800));

    /**
     * Order corners in consistent sequence: top-left, top-right, bottom-right, bottom-left
     * @param corners Unordered corner points
     * @return Ordered corner points
     */
    std::vector<cv::Point2f> orderCorners(const std::vector<cv::Point2f> &corners);

    /**
     * Apply perspective transformation
     * @param input Source image
     * @param srcCorners Source quadrilateral corners (ordered)
     * @param targetSize Target rectangle size
     * @return Warped image
     */
    cv::Mat warpPerspective(const cv::Mat &input,
                            const std::vector<cv::Point2f> &srcCorners,
                            const cv::Size &targetSize);

private:
    /**
     * Calculate distance between two points
     */
    double calculateDistance(const cv::Point2f &p1, const cv::Point2f &p2);

    /**
     * Find sum and difference of coordinates for ordering
     */
    cv::Point2f calculateSumAndDiff(const cv::Point2f &point);
};