// include/visualization.h
#ifndef VISUALIZATION_H_
#define VISUALIZATION_H_

#include <opencv2/opencv.hpp>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;
using cv::Mat;
using cv::Point2f;

/**
 * Visualization functions for drawing detection results.
 */

/**
 * Draws detected and ground truth boxes on image and saves result.
 */
void drawBoxes(const Mat &img, const std::vector<Point2f> &detected,
               const std::vector<Point2f> &gt, const fs::path &outputPath);

#endif // VISUALIZATION_H_