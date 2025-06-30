// src/image_preprocessing.cpp
#include "image_preprocessing.h"
#include <vector>
#include <algorithm>

using cv::Mat;
using cv::Point2f;

double preprocessImage(const Mat &img, Mat &mag, Mat &eq)
{
    // Convert to grayscale and apply CLAHE (advanced histogram equalization)
    Mat gray;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    cv::createCLAHE(3.875, cv::Size(9, 9))->apply(gray, eq);

    // Sobel filter (x gradient, y gradient)
    Mat sx, sy;
    cv::Sobel(eq, sx, CV_32F, 1, 0);
    cv::Sobel(eq, sy, CV_32F, 0, 1);

    // Compute gradient magnitude image
    cv::magnitude(sx, sy, mag);

    // Normalize to 8 bits and threshold
    double mx;
    // Maximum gradient value
    cv::minMaxLoc(mag, 0, &mx);
    mag.convertTo(mag, CV_8U, 255.0 / (mx + 1e-3));

    // Apply Otsu's threshold to binarize gradient image (two classes)
    cv::threshold(mag, mag, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    // Apply morphological closing to fill small gaps in edges
    cv::morphologyEx(mag, mag, cv::MORPH_CLOSE, Mat(), {-1, -1}, 4);

    // Calculate median gradient using all non-zero pixel values in mag
    std::vector<uchar> v;
    v.reserve(mag.total());
    for (int r = 0; r < mag.rows; r++)
    {
        for (int c = 0; c < mag.cols; c++)
        {
            if (mag.at<uchar>(r, c))
            {
                v.push_back(mag.at<uchar>(r, c));
            }
        }
    }

    // Return median non-zero gradient
    return v.empty() ? 0 : v[v.size() / 2];
}
