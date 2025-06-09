// src/evaluation.cpp
#include "evaluation.h"
#include <cmath>

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