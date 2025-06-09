// include/contour_analysis.h
#ifndef CONTOUR_ANALYSIS_H_
#define CONTOUR_ANALYSIS_H_

#include <opencv2/opencv.hpp>
#include <vector>

using cv::Mat;
using cv::Point2f;

/**
 * Functions for analyzing contour quality and scoring.
 */

/**
 * Calculates mean edge strength along quadrilateral edges.
 */
double edgeMean(const std::vector<Point2f> &q, const Mat &eq);

/**
 * Calculates fraction of quadrilateral perimeter touching image borders.
 */
double borderFrac(const std::vector<Point2f> &q, int W, int H);

/**
 * Calculates whiteness score comparing document interior to background.
 */
double whiteness(const std::vector<Point2f> &q, const Mat &gray);

/**
 * Candidate structure for quadrilateral scoring.
 */
struct Cand
{
    std::vector<Point2f> q;
    double sc;
};

/**
 * Evaluates a quadrilateral and adds it to candidate list if valid.
 */
void evalQuad(std::vector<Point2f> q, std::vector<Cand> &list,
              double Aimg, int W, int H, const Mat &eq, const Mat &gray, double medGrad);

#endif // CONTOUR_ANALYSIS_H_