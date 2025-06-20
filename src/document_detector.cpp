// src/document_detector.cpp
#include "document_detector.h"
#include "image_preprocessing.h"
#include "contour_analysis.h"
#include "geometry_utils.h"
#include <algorithm>

std::vector<Point2f> detect(const Mat &img)
{
    int W = img.cols, H = img.rows;
    double Aimg = img.total();

    // Preprocess image
    Mat mag, eq;
    double medGrad = preprocessImage(img, mag, eq);

    // Find contours
    std::vector<std::vector<cv::Point>> C;
    cv::findContours(mag, C, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

    // Convert to grayscale for scoring functions
    Mat gray;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

    std::vector<Cand> list;

    // 1. Polygon approximation with 4 sides
    for (auto &cont : C)
    {
        std::vector<cv::Point> ap;
        cv::approxPolyDP(cont, ap, 0.005 * cv::arcLength(cont, true), true);
        if (ap.size() == 4 && cv::isContourConvex(ap))
        {
            std::vector<Point2f> q(ap.begin(), ap.end());
            orderCCW(q);
            evalQuad(q, list, Aimg, W, H, eq, gray, medGrad);
        }
    }

    // 2. Minimum area rectangle for each contour
    for (auto &cont : C)
    {
        cv::RotatedRect rr = cv::minAreaRect(cont);
        Point2f r[4];
        rr.points(r);
        std::vector<Point2f> q(r, r + 4);
        orderCCW(q);
        evalQuad(q, list, Aimg, W, H, eq, gray, medGrad);
    }

    // Choose best score
    std::vector<Point2f> best;
    if (!list.empty())
    {
        auto top = *std::max_element(list.begin(), list.end(),
                                     [](auto &a, auto &b)
                                     { return a.sc < b.sc; });
        if (top.sc >= 0.3)
            best = top.q;
    }

    if (best.empty())
    {
        // Minimum area rectangle of largest contour
        auto &big = *std::max_element(C.begin(), C.end(), [](auto &a, auto &b)
                                      { return fabs(cv::contourArea(a)) < fabs(cv::contourArea(b)); });
        cv::RotatedRect rr = cv::minAreaRect(big);
        Point2f r[4];
        rr.points(r);
        best.assign(r, r + 4);
        orderCCW(best);
    }

    // Refine ±2 px if border safe
    if (borderFrac(best, W, H) < 0.2)
    {
        for (int i = 0; i < 4; i++)
        {
            auto d = best[(i + 1) & 3] - best[i];
            double L = cv::norm(d);
            if (L > 0)
            {
                cv::Point2f n(d.y / L, -d.x / L);
                best[i] -= 2 * n;
                best[(i + 1) & 3] += 2 * n;
            }
        }
    }

    for (auto &p : best)
        clipPt(p, W, H);
    return best;
}