// src/geometry_utils.cpp
#include "geometry_utils.h"
#include <algorithm>

using cv::Mat;
using cv::Point2f;

// Check if quad intersects itself
bool crossSelf(const std::vector<Point2f> &q)
{
    // Lambda function to find cross product
    auto z = [](Point2f a, Point2f b, Point2f c)
    {
        return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    };

    // Check for intersection between edges (0.1) and (2-3)
    return z(q[0], q[1], q[2]) * z(q[0], q[1], q[3]) < 0 &&
           z(q[2], q[3], q[0]) * z(q[2], q[3], q[1]) < 0;
}

// Make sure a point remains withing image bounds
void clipPt(Point2f &p, int W, int H)
{
    p.x = std::clamp(p.x, 0.f, (float)(W - 1));
    p.y = std::clamp(p.y, 0.f, (float)(H - 1));
}

// Order points counter clock wise (top left, bottom left, bottom right, top right)
void orderCCW(std::vector<Point2f>& q)
{
    if (q.size() != 4)
    {
        std::cerr << "orderCCW expects 4 points, got " << q.size() << '\n';
        return;
    }

    // Find the centroid of a quad
    Point2f c(0,0);
    for (auto& p : q) c += p;
    c *= 0.25f;

    // Sort by angle relative to a centroid
    std::sort(q.begin(), q.end(),
              [&](const auto& a, const auto& b)
              { return std::atan2(a.y-c.y, a.x-c.x) <
                       std::atan2(b.y-c.y, b.x-c.x); });

    // Rotate so that top-left points is first
    size_t tl = 0;
    for (size_t i = 1; i < 4; ++i)
        if (q[i].y < q[tl].y || (q[i].y == q[tl].y && q[i].x < q[tl].x))
            tl = i;
    std::rotate(q.begin(), q.begin()+tl, q.end());
}

// Same CCW ordering, kept in the code for convenience
// Returns ordered copy
std::vector<cv::Point2f> orderPoints(const std::vector<cv::Point2f>& pts)
{
    std::vector<cv::Point2f> tmp = pts;   // make a copy
    orderCCW(tmp);                        // in-place CCW + TL start
    return tmp;                           // return ordered copy
}

// Transform from image view to top down view
cv::Mat fourPointTransform(const cv::Mat& image, const std::vector<cv::Point2f>& pts)
{
    // Read points clock wise
    std::vector<cv::Point2f> rect = orderPoints(pts);
    const cv::Point2f& tl = rect[0];
    const cv::Point2f& tr = rect[1];
    const cv::Point2f& br = rect[2];
    const cv::Point2f& bl = rect[3];

    // Width of the new image = max distance either at bottom or top side
    float widthA = cv::norm(br - bl);
    float widthB = cv::norm(tr - tl);
    int maxWidth = static_cast<int>(std::max(widthA, widthB));

    // Height of the new image = max distance either at right or left side
    float heightA = cv::norm(tr - br);
    float heightB = cv::norm(tl - bl);
    int maxHeight = static_cast<int>(std::max(heightA, heightB));

    // Set corners for the warped rectangle
    std::vector<cv::Point2f> dst = {
        {0.0f, 0.0f},
        {static_cast<float>(maxWidth - 1), 0.0f},
        {static_cast<float>(maxWidth - 1), static_cast<float>(maxHeight - 1)},
        {0.0f, static_cast<float>(maxHeight - 1)}
    };

    // Compute perspective transform matrix
    cv::Mat M = cv::getPerspectiveTransform(rect, dst);

    // Warp the image
    cv::Mat warped;
    cv::warpPerspective(image, warped, M, cv::Size(maxWidth, maxHeight));

    return warped;
}
