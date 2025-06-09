// src/contour_analysis.cpp
#include "contour_analysis.h"
#include "geometry_utils.h"
#include <cmath>

double edgeMean(const std::vector<Point2f> &q, const Mat &eq)
{
    Mat sob;
    cv::Sobel(eq, sob, CV_32F, 1, 1);
    double s = 0;
    int n = 0;

    for (int i = 0; i < 4; i++)
    {
        cv::LineIterator it(sob, q[i], q[(i + 1) & 3], 8);
        for (int j = 0; j < it.count; ++j, ++it)
        {
            s += static_cast<double>((*it)[0]); // Remove abs since uchar is always positive
            ++n;
        }
    }
    return n ? s / n : 0;
}

double borderFrac(const std::vector<Point2f> &q, int W, int H)
{
    double touch = 0, per = 0;
    for (int i = 0; i < 4; i++)
    {
        auto a = q[i], b = q[(i + 1) & 3];
        double len = cv::norm(a - b);
        per += len;
        if ((a.x < 2 && b.x < 2) || (a.x > W - 3 && b.x > W - 3) ||
            (a.y < 2 && b.y < 2) || (a.y > H - 3 && b.y > H - 3))
        {
            touch += len;
        }
    }
    return touch / per;
}

double whiteness(const std::vector<Point2f> &q, const Mat &gray)
{
    std::vector<cv::Point> poly;
    for (auto &p : q)
        poly.emplace_back(p);

    Mat mask = Mat::zeros(gray.size(), CV_8U);
    cv::fillConvexPoly(mask, poly, 255);

    cv::Scalar mDoc = cv::mean(gray, mask);
    cv::Scalar mBg = cv::mean(gray, 255 - mask);

    double w = (mBg[0] > 1) ? mDoc[0] / mBg[0] : 1;
    if (w < 1)
        w = 1 / w;
    return std::clamp((w - 1) / 0.5, 0.0, 1.0);
}

void evalQuad(std::vector<Point2f> q, std::vector<Cand> &list,
              double Aimg, int W, int H, const Mat &eq, const Mat &gray, double medGrad)
{
    if (crossSelf(q))
        return;

    double A = fabs(cv::contourArea(q));
    if (A < 0.029 * Aimg)
        return;

    double bF = borderFrac(q, W, H);
    if (bF > 0.476)
        return;

    double areaFit = 1 - std::abs(A - 0.458 * Aimg) / (0.458 * Aimg);

    double ar = std::max(cv::norm(q[0] - q[1]), cv::norm(q[1] - q[2])) /
                std::min(cv::norm(q[0] - q[1]), cv::norm(q[1] - q[2]));
    double ARfit = 1 - std::min(std::abs(ar - 1.414) / 1.0, 1.0);

    double gradFit = medGrad > 1 ? std::clamp(edgeMean(q, eq) / (edgeMean(q, eq) + medGrad), 0.0, 1.0) : 0.5;
    double wFit = whiteness(q, gray);

    // Optimized weights
    double score = 0.329 * areaFit + 0.266 * wFit + 0.208 * gradFit + 0.197 * ARfit;
    list.push_back({q, score});
}