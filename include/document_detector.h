// include/document_detector.h
#ifndef DOCUMENT_DETECTOR_H_
#define DOCUMENT_DETECTOR_H_

#include <opencv2/opencv.hpp>
#include <vector>

// Main document detection function
// Detects document corners in the input image
std::vector<cv::Point2f> detect(const cv::Mat &img);

#endif // DOCUMENT_DETECTOR_H_