// include/image_preprocessing.h
#ifndef IMAGE_PREPROCESSING_H_
#define IMAGE_PREPROCESSING_H_

#include <opencv2/opencv.hpp>

// Image preprocessing functions for edge detection and enhancement

// Preprocesses image and returns median gradient value
// Applies CLAHE, Sobel operators, thresholding, and morphological operations

double preprocessImage(const cv::Mat &img, cv::Mat &mag, cv::Mat &eq);

#endif // IMAGE_PREPROCESSING_H_