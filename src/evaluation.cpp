// src/evaluation.cpp
#include "evaluation.h"
#include <cmath>

using cv::Mat;
using cv::Point2f;

double IoU(std::vector<Point2f> a, std::vector<Point2f> b)
{
    if (a.size() != 4 || b.size() != 4)
        return -1;

    std::vector<cv::Point> A, B;
    for (auto &p : a)
        A.emplace_back(p);
    for (auto &p : b)
        B.emplace_back(p);

    double a1 = fabs(cv::contourArea(A));
    double a2 = fabs(cv::contourArea(B));

    cv::Mat inter;
    bool ok = cv::intersectConvexConvex(A, B, inter);
    double ai = (ok && !inter.empty()) ? fabs(cv::contourArea(inter)) : 0;

    return (a1 + a2 - ai) > 1e-5 ? ai / (a1 + a2 - ai) : 0;
}


double averagePointDistance(const std::vector<cv::Point2f>& a, const std::vector<cv::Point2f>& b)
{
    if (a.size() != b.size()) throw std::invalid_argument("Point sets must have same size");
    
    double totalDist = 0.0;
    for (size_t i = 0; i < a.size(); ++i) {
        double dx = a[i].x - b[i].x;
        double dy = a[i].y - b[i].y;
        // Euclidean distance between the two points
        totalDist += std::sqrt(dx*dx + dy*dy);
    }
    return totalDist / a.size();
}