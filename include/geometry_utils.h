// include/geometry_utils.h
#ifndef GEOMETRY_UTILS_H_
#define GEOMETRY_UTILS_H_

#include <opencv2/opencv.hpp>
#include <vector>

namespace fs = std::filesystem;

// Utility functions for geometric operations

// Orders four points in counter-clockwise order starting from top-left
void orderCCW(std::vector<cv::Point2f> &q);

// Checks if a quadrilateral is self-intersecting

bool crossSelf(const std::vector<cv::Point2f> &q);

// Clips a point to be within image boundaries
void clipPt(cv::Point2f &p, int W, int H);

#endif // GEOMETRY_UTILS_H_