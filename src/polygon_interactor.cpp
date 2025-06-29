/*  polygon_interactor.cpp  ----------------------------------------- */
#include "polygon_interactor.h"
#include "geometry_utils.h"          // fourPointTransform
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <sstream>
#include <iomanip>
#include <filesystem>

PolygonInteractor::PolygonInteractor(const cv::Mat& img,
                                     const std::vector<cv::Point2f>& quad,
                                     const std::string& outDir)
    : original_(img.clone()),
      display_(img.clone()),
      quad_(quad),
      activeIdx_(-1),
      eps2_(25.0),                   // epsilon = 5 px → eps² = 25
      outDir_(outDir)
{
    if (quad_.size() != 4)
        throw std::runtime_error("PolygonInteractor expects a quad of 4 points");
    drawOverlay();
}

std::vector<cv::Point2f> PolygonInteractor::editedQuad() const
{
    return quad_;
}

void PolygonInteractor::drawOverlay()
{
    display_ = original_.clone();

    /* 1. draw polygon edges & corner handles -------------------------------- */
    const cv::Scalar red  (  0,   0, 255);
    const cv::Scalar green(  0, 255,   0);

    for (int i = 0; i < 4; ++i)
        cv::line(display_, quad_[i], quad_[(i+1)&3], green, 2, cv::LINE_AA);

    for (int i = 0; i < 4; ++i)
        cv::circle(display_, quad_[i], 4, red, cv::FILLED, cv::LINE_AA);

    /* 2. *** user hint *** --------------------------------------------------- */
    cv::putText(display_,
                "Press 's' to save the scan",          // text
                cv::Point(15, 30),                     // position (x,y)
                cv::FONT_HERSHEY_SIMPLEX, 0.9,         // font & scale
                cv::Scalar(255, 255, 255), 2,          // colour & thickness
                cv::LINE_AA);

    cv::imshow("Adjust document", display_);
}

int PolygonInteractor::hitTest(const cv::Point2f& p) const
{
    for (int i = 0; i < 4; ++i)
    {
        cv::Point2f d = p - quad_[i];
        double dist2 = d.x*d.x + d.y*d.y;
        if (dist2 <= eps2_) return i;
    }
    return -1;
}

void PolygonInteractor::onMouse(int event, int x, int y, int /*flags*/, void* userdata)
{
    PolygonInteractor* self = reinterpret_cast<PolygonInteractor*>(userdata);
    if (!self) return;

    cv::Point2f pt(static_cast<float>(x), static_cast<float>(y));

    switch (event)
    {
    case cv::EVENT_LBUTTONDOWN:
        self->activeIdx_ = self->hitTest(pt);
        break;

    case cv::EVENT_MOUSEMOVE:
    if (self->activeIdx_ >= 0)
    {
        self->quad_[self->activeIdx_] = pt;   // change ONLY the dragged point
        orderCCW(self->quad_);               // keep CCW & TL-first
        self->drawOverlay();
    }
    break;

    case cv::EVENT_LBUTTONUP:
    self->activeIdx_ = -1;
    orderCCW(self->quad_);                   // final tidy-up
    self->drawOverlay();
    break;

    default:
        break;
    }
}

void PolygonInteractor::run()
{
    const std::string winName("Adjust document");
    cv::namedWindow(winName, cv::WINDOW_AUTOSIZE);
    cv::setMouseCallback(winName, PolygonInteractor::onMouse, this);
    drawOverlay();

    for (;;)
    {
        int key = cv::waitKey(20);
        if (key == 27)         // ESC → quit without saving
            break;

        if (key == 's' || key == 'S')
        {
            // Warp and save
            cv::Mat scan = fourPointTransform(original_, quad_);
            std::filesystem::create_directories(outDir_);
            std::ostringstream fname;
            fname << outDir_ << "/scan_" << std::setw(6) << std::setfill('0')
                  << std::rand() % 1000000 << ".png";
            cv::imwrite(fname.str(), scan);
            std::cout << "Saved scan to: " << fname.str() << std::endl;
            break;
        }
    }
    cv::destroyWindow(winName);
}

