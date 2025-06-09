#ifndef FILE_IO_H_
#define FILE_IO_H_

#include <opencv2/opencv.hpp>
#include <filesystem>
#include <vector>
#include <string>
#include <sstream>

namespace fs = std::filesystem;
using cv::Point2f;

/**
 * File I/O operations for coordinates and ground truth data.
 */

/**
 * Saves quadrilateral coordinates to text file.
 */
void saveTxt(const fs::path& p, const std::vector<Point2f>& q);

/**
 * Reads ground truth coordinates from the specific coordinates.txt format.
 * Format: img_X: "x1 y1"	"x2 y2" 	"x3 y3" 	"x4 y4"
 */
std::vector<Point2f> readGtFromCoordinatesFile(const fs::path& coordFile, const std::string& imageName);

/**
 * Reads ground truth coordinates from text file.
 */
std::vector<Point2f> readGt(const fs::path& t);

#endif  // FILE_IO_H_
