// include/evaluation.h
#ifndef EVALUATION_H_
#define EVALUATION_H_

#include <opencv2/opencv.hpp>
#include <vector>

using cv::Mat;
using cv::Point2f;

// Evaluation metrics for document detection accuracy

// Calculates Intersection over Union (IoU) between two quadrilaterals
double IoU(std::vector<cv::Point2f> a, std::vector<cv::Point2f> b);

// Computes average distance between detected corners and true corners
double averagePointDistance(const std::vector<cv::Point2f>& a, const std::vector<cv::Point2f>& b);

#endif // EVALUATION_H_