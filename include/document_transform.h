// include/document_transform.h
#ifndef DOCUMENT_TRANSFORM_H_
#define DOCUMENT_TRANSFORM_H_

#include <opencv2/opencv.hpp>
#include <vector>
#include <algorithm>
#include <cmath>

// Order rectangle points clock wise (tl, tr, br, bl)
std::vector<cv::Point2f> orderPoints(const std::vector<cv::Point2f>& pts);


// Four points warping to achieve final scan of the document
cv::Mat fourPointTransform(const cv::Mat& image, const std::vector<cv::Point2f>& pts);

#endif // DOCUMENT_TRANSFORM_H_