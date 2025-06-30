// src/visualization.cpp
#include "visualization.h"

using cv::Mat;
using cv::Point2f;

void drawBoxes(const Mat &img, const std::vector<Point2f> &detected,
               const std::vector<Point2f> &gt, const fs::path &outputPath)
{
    // Clone original image for annotations
    Mat result = img.clone();

    // Draw ground truth box in green
    if (gt.size() == 4)
    {
        std::vector<cv::Point> gtPoly;
        for (const auto &p : gt)
        {
            gtPoly.push_back(cv::Point((int)p.x, (int)p.y));
        }
        cv::polylines(result, gtPoly, true, cv::Scalar(0, 255, 0), 3); // Green

        // Add GT label
        cv::putText(result, "GT", cv::Point((int)gt[0].x, (int)gt[0].y - 10),
                    cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);
    }

    // Draw detected box in red
    if (detected.size() == 4)
    {
        std::vector<cv::Point> detPoly;
        for (const auto &p : detected)
        {
            // Convert to int points
            detPoly.push_back(cv::Point((int)p.x, (int)p.y));
        }
        cv::polylines(result, detPoly, true, cv::Scalar(0, 0, 255), 3); // Red

        // Add labels
        cv::putText(result, "DET", cv::Point((int)detected[0].x, (int)detected[0].y - 10),
                    cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 255), 2);
    }

    // Save result image
    fs::create_directories(outputPath.parent_path());
    cv::imwrite(outputPath.string(), result);
}