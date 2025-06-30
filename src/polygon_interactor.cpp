// src/polygon_interactor.cpp
#include "polygon_interactor.h"
#include "geometry_utils.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <sstream>
#include <iomanip>
#include <filesystem>

PolygonInteractor::PolygonInteractor(const cv::Mat& img,
                                     const std::vector<cv::Point2f>& quad,
                                     const std::string& outDir)
    : original_(img.clone()),
      display_(img.clone()),    // Copy to include overlays
      quad_(quad),
      activeIdx_(-1),           // Index of the dragged point
      eps2_(25.0),              // Epsilon (drag radius) = 5.0 px -> eps^2 = 25.0
      outDir_(outDir)
{
    if (quad_.size() != 4)
        throw std::runtime_error("PolygonInteractor expects a quad of 4 points");
    // Draw UI elements on the image
    drawOverlay();
}

// Return the adjusted quad
std::vector<cv::Point2f> PolygonInteractor::editedQuad() const
{
    return quad_;
}

// Draw lines, corner circles and hint text
void PolygonInteractor::drawOverlay()
{
    // Reset to clean copy
    display_ = original_.clone();

    const cv::Scalar red  (  0,   0, 255);
    const cv::Scalar green(  0, 255,   0);

    // Draw polygon edges
    for (int i = 0; i < 4; ++i)
        cv::line(display_, quad_[i], quad_[(i+1)&3], green, 2, cv::LINE_AA);

    // Draw ccircular orner handles
    for (int i = 0; i < 4; ++i)
        cv::circle(display_, quad_[i], 4, red, cv::FILLED, cv::LINE_AA);

    // Draw instructions
    cv::putText(display_,
                "Press 's' to save the scan",
                cv::Point(15, 30),
                cv::FONT_HERSHEY_SIMPLEX, 0.9,
                cv::Scalar(255, 255, 255), 2,
                cv::LINE_AA);

    // Insert text in the displayed image
    cv::imshow("Adjust document", display_);
}

// Returns index of closest corner within drag radius
int PolygonInteractor::hitTest(const cv::Point2f& p) const
{
    for (int i = 0; i < 4; ++i)
    {
        cv::Point2f d = p - quad_[i];
        double dist2 = d.x*d.x + d.y*d.y;
        if (dist2 <= eps2_) return i;
    }
    // No corner hit
    return -1;
}

// Mouse callback handler (drag/drop corner points)
void PolygonInteractor::onMouse(int event, int x, int y, int /*flags*/, void* userdata)
{
    PolygonInteractor* self = reinterpret_cast<PolygonInteractor*>(userdata);
    if (!self) return;

    cv::Point2f pt(static_cast<float>(x), static_cast<float>(y));

    switch (event)
    {
    // Mouse button pressed
    case cv::EVENT_LBUTTONDOWN:
        // Check if click was close to a point
        self->activeIdx_ = self->hitTest(pt);
        break;

    // Mouse moved
    case cv::EVENT_MOUSEMOVE:
    if (self->activeIdx_ >= 0)
    {
        // Drag point (do not influence other points)
        self->quad_[self->activeIdx_] = pt;
        // Keep corners ordered CCW
        orderCCW(self->quad_);
        // Update display
        self->drawOverlay();
    }
    break;

    // Released mouse click
    case cv::EVENT_LBUTTONUP:
    // Release dragged corner
    self->activeIdx_ = -1;
    // Re-order points to clean up
    orderCCW(self->quad_);
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

    // Show initial poly
    drawOverlay();

    for (;;)
    {
        int key = cv::waitKey(20);
        // ESC: quit without saving
        if (key == 27)
            break;

        if (key == 's' || key == 'S')
        {
            // Warp image to get final scan and save result
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

