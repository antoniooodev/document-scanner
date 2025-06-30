/*  polygon_interactor.h  ------------------------------------------- */
#ifndef POLYGON_INTERACTOR_H_
#define POLYGON_INTERACTOR_H_

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

class PolygonInteractor
{
public:
    // img  : image shown to the user
    // quad : 4-point document quad (must be CCW, TL first)
    // outDir : where to store the warped scan when user presses 's'
    PolygonInteractor(const cv::Mat& img,
                      const std::vector<cv::Point2f>& quad,
                      const std::string& outDir);

    // Launch the interactive window (blocks until user finishes)
    void run();

    // The (possibly) edited quad can be fetched afterwards
    std::vector<cv::Point2f> editedQuad() const;

private:
    // helper methods
    void drawOverlay();
    // returns vertex index or -1
    int  hitTest(const cv::Point2f& p) const;
    static void onMouse(int event, int x, int y, int flags, void* userdata);

    // data members
    cv::Mat                 original_;    // original image
    cv::Mat                 display_;     // image with overlay
    std::vector<cv::Point2f> quad_;       // editable quad (size 4)
    int                     activeIdx_;   // index of vertex being dragged, -1 else
    double                  eps2_;        // squared hit radius (epsilon^2)
    std::string             outDir_;      // where to save the scan
};

#endif // POLYGON_INTERACTOR_H_

