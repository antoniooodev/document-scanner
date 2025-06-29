// include/geometry_utils.h
#ifndef GEOMETRY_UTILS_H_
#define GEOMETRY_UTILS_H_

#include <opencv2/opencv.hpp>
#include <vector>
#include <algorithm>
#include <cmath>

namespace fs = std::filesystem;

// Utility functions for geometric operations

// Checks if a quadrilateral is self-intersecting

bool crossSelf(const std::vector<cv::Point2f> &q);

// Clips a point to be within image boundaries
void clipPt(cv::Point2f &p, int W, int H);

// Order quad points clock wise (tl, tr, br, bl)
void orderCCW(std::vector<cv::Point2f>& q);

std::vector<cv::Point2f> orderPoints(const std::vector<cv::Point2f>& pts);

// Four points warping to achieve final scan of the document
cv::Mat fourPointTransform(const cv::Mat& image, const std::vector<cv::Point2f>& pts);


#endif // GEOMETRY_UTILS_H_
