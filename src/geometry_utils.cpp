// src/geometry_utils.cpp
#include "geometry_utils.h"
#include <algorithm>

using cv::Mat;
using cv::Point2f;

void orderCCW(std::vector<Point2f> &q)
{
    if (q.size() != 4)
    {
        std::cerr << "Error: orderCCW expects exactly 4 points, got " << q.size() << std::endl;
        return;
    }

    Point2f c(0, 0);
    for (auto &p : q)
        c += p;
    c *= 0.25f;

    std::sort(q.begin(), q.end(), [&](const auto &a, const auto &b)
              { return std::atan2(a.y - c.y, a.x - c.x) < std::atan2(b.y - c.y, b.x - c.x); });

    size_t tl = 0;
    for (size_t i = 1; i < 4; i++)
    {
        if (q[i].y < q[tl].y || (q[i].y == q[tl].y && q[i].x < q[tl].x))
        {
            tl = i;
        }
    }
    std::rotate(q.begin(), q.begin() + tl, q.end());
}

bool crossSelf(const std::vector<Point2f> &q)
{
    auto z = [](Point2f a, Point2f b, Point2f c)
    {
        return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    };
    return z(q[0], q[1], q[2]) * z(q[0], q[1], q[3]) < 0 &&
           z(q[2], q[3], q[0]) * z(q[2], q[3], q[1]) < 0;
}

void clipPt(Point2f &p, int W, int H)
{
    p.x = std::clamp(p.x, 0.f, (float)(W - 1));
    p.y = std::clamp(p.y, 0.f, (float)(H - 1));
}