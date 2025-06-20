// src/document_transform.cpp
#include "document_transform.h"


std::vector<cv::Point2f> orderPoints(const std::vector<cv::Point2f>& pts)
{
    std::vector<cv::Point2f> xSorted = pts;

    // Sort points by x-coordinate
    std::sort(xSorted.begin(), xSorted.end(), [](const cv::Point2f& a, const cv::Point2f& b) {
        return a.x < b.x;
    });

    // Left most = first two points, right most = last two points
    std::vector<cv::Point2f> leftMost(xSorted.begin(), xSorted.begin() + 2);
    std::vector<cv::Point2f> rightMost(xSorted.begin() + 2, xSorted.end());

    // Sort left-most by y to get top left and bottom left
    std::sort(leftMost.begin(), leftMost.end(), [](const cv::Point2f& a, const cv::Point2f& b) {
        return a.y < b.y;
    });
    cv::Point2f tl = leftMost[0];
    cv::Point2f bl = leftMost[1];

    // Use top left as a pin point and choose bottom right as the farthest point from it on right side
    double dist1 = cv::norm(tl - rightMost[0]);
    double dist2 = cv::norm(tl - rightMost[1]);

    cv::Point2f br, tr;
    if (dist1 > dist2) {
        br = rightMost[0];
        tr = rightMost[1];
    } else {
        br = rightMost[1];
        tr = rightMost[0];
    }

    // Return points ordered clock wise
    return {tl, tr, br, bl};
}


cv::Mat fourPointTransform(const cv::Mat& image, const std::vector<cv::Point2f>& pts)
{
    // Read points clock wise
    std::vector<cv::Point2f> rect = orderPoints(pts);
    const cv::Point2f& tl = rect[0];
    const cv::Point2f& tr = rect[1];
    const cv::Point2f& br = rect[2];
    const cv::Point2f& bl = rect[3];

    // Width of the new image = max distance either at bottom or top side
    float widthA = cv::norm(br - bl);
    float widthB = cv::norm(tr - tl);
    int maxWidth = static_cast<int>(std::max(widthA, widthB));

    // Height of the new image = max distance either at right or left side
    float heightA = cv::norm(tr - br);
    float heightB = cv::norm(tl - bl);
    int maxHeight = static_cast<int>(std::max(heightA, heightB));

    // Set corners for the warped rectangle
    std::vector<cv::Point2f> dst = {
        {0.0f, 0.0f},
        {static_cast<float>(maxWidth - 1), 0.0f},
        {static_cast<float>(maxWidth - 1), static_cast<float>(maxHeight - 1)},
        {0.0f, static_cast<float>(maxHeight - 1)}
    };

    // Compute perspective transform matrix
    cv::Mat M = cv::getPerspectiveTransform(rect, dst);

    // Warp the image
    cv::Mat warped;
    cv::warpPerspective(image, warped, M, cv::Size(maxWidth, maxHeight));

    return warped;
}