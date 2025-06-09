// include/evaluation.h
#ifndef EVALUATION_H_
#define EVALUATION_H_

#include <opencv2/opencv.hpp>
#include <vector>

using cv::Point2f;

/**
 * Evaluation metrics for document detection accuracy.
 */

/**
 * Calculates Intersection over Union (IoU) between two quadrilaterals.
 */
double IoU(std::vector<Point2f> a, std::vector<Point2f> b);

#endif // EVALUATION_H_