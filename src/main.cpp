// src/main.cpp
#include "document_detector.h"
#include "file_io.h"
#include "evaluation.h"
#include "visualization.h"
#include "geometry_utils.h"
#include "polygon_interactor.h"

#include <opencv2/opencv.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <chrono>
#include <cstring>

namespace fs = std::filesystem;
using cv::Mat;
using cv::Point2f;

// SINGLE-IMG
// exec – run detection on a single image and append result line to log
static void exec(const fs::path &imgP,
                 const fs::path &gtP,
                 bool            interactive,
                 double         &iou,
                 double         &dist,
                 double         &ms)
{
    // Start processing timer
    const auto tic = std::chrono::high_resolution_clock::now();

    // Load original image
    Mat src = cv::imread(imgP.string());
    if (src.empty())
        throw std::runtime_error("imread failed");

    // Resize image for faster processing
    const double sc = 600.0 / std::max(src.cols, src.rows);
    Mat mini;
    cv::resize(src, mini, cv::Size(), sc, sc, cv::INTER_AREA);

    // Run quad detection on resized image
    std::vector<Point2f> quad = detect(mini);
    // Ensure all points are within image bound
    for (Point2f &p : quad)
        clipPt(p, mini.cols, mini.rows);

    // If user selected the interactive function, start GUI
    if (interactive)
    {
        PolygonInteractor gui(mini, quad, "data/output");
        gui.run();
        quad = gui.editedQuad();
    }

    // Keep CCW ordering on points
    quad = orderPoints(quad);

    // Scale points back to original scade for higher quality scan
    std::vector<Point2f> quadFull;
    quadFull.reserve(4);
    for (const Point2f &p : quad)
        quadFull.emplace_back(p.x / sc, p.y / sc);

    // Warp the document to get the final scan
    Mat warped = fourPointTransform(src, quadFull);

    // Ground truth
    std::vector<Point2f> gt;
    // Init performance values
    iou  = -1.0;
    dist = -1.0;

    if (!gtP.empty() && fs::exists(gtP))
    {
        const std::string imgName = imgP.stem().string();

        // Start by trying common coordinates file for GT
        gt = readGtFromCoordinatesFile(gtP, imgName);
        if (!gt.empty())
        {
            // Rescale to resized image scale
            for (Point2f &p : gt)
            {
                p.x *= static_cast<float>(sc);
                p.y *= static_cast<float>(sc);
            }
        }
        else
        // Fallback to (x,y) format for gt
        {
            gt = readGt(gtP);
            for (Point2f &p : gt)
            {
                p.x = (p.x / 449.0f) * (mini.cols - 1);
                p.y = (p.y / 599.0f) * (mini.rows - 1);
            }
        }

        // If ground truth is available, evaluate IoU and point distance
        if (!gt.empty())
        {
            for (Point2f &p : gt)
                clipPt(p, mini.cols, mini.rows);
            gt = orderPoints(gt);
            iou  = IoU(quad, gt);
            dist = averagePointDistance(quad, gt);
        }
    }

    // Save visual results: box overlay on original image and document scan
    fs::create_directories("data/output");
    fs::path visP  = "data/output/" + imgP.stem().string() + "_boxes.png";
    fs::path scanP = "data/output/" + imgP.stem().string() + "_scan.png";

    drawBoxes(mini, quad, gt, visP);
    cv::imwrite(scanP.string(), warped);

    // Measure processing time
    ms = std::chrono::duration<double, std::milli>(
             std::chrono::high_resolution_clock::now() - tic)
             .count();

    // Console log message with performance
    std::cout << imgP.filename() << "  IoU=" << iou
              << "  AvgDist=" << dist << " px  Time=" << ms << " ms\n";

    // File log with performance
    std::ofstream("data/output/complete_results.txt", std::ios::app)
        << imgP.filename() << "  IoU=" << iou
        << "  AvgDist=" << dist
        << "  Time=" << ms << " ms\n";
}

// Dataset mode / Single image mode
// GUI Interactor if needed
int main(int argc, char **argv)
{
    // 
    fs::create_directories("data/output");
    std::ofstream("data/output/complete_results.txt", std::ios::trunc).close();

    if (argc < 2)
    {
        std::cout << "Usage:\n  ./DocumentScanner [-i] image.png [gt.txt]\n  ./DocumentScanner [-i] --dataset DIR\n";
        return 0;
    }

    // GUI mode (flag)
    bool interactive = false;

    std::vector<std::string> pos;

    for (int i = 1; i < argc; ++i)
    {
        if (!std::strcmp(argv[i], "-i"))
            interactive = true;
        else
            pos.emplace_back(argv[i]);
    }
    if (pos.empty())
    {
        std::cerr << "No image or dataset given.\n";
        return 1;
    }

    // Dataset mode
    if (pos[0] == "--dataset")
    {
        if (pos.size() < 2)
        {
            std::cerr << "--dataset needs DIR\n";
            return 1;
        }
        // Dataset folder
        fs::path dir       = pos[1];
        fs::path coordFile = dir / "../ground_truth/coordinates.txt";

        double sumIoU = 0.0, sumDist = 0.0, sumMs = 0.0;
        int    n      = 0;

        // Process img_1 to img_10
        // .jpg or .png auto handled
        for (int k = 1; k <= 10; ++k)
        {
            fs::path img = dir / ("img_" + std::to_string(k) + ".png");
            if (!fs::exists(img))
                img = dir / ("img_" + std::to_string(k) + ".jpg");
            if (!fs::exists(img))
                continue;

            double iou, dist, ms;
            try
            {
                exec(img, coordFile, interactive, iou, dist, ms);
                ++n;
                sumIoU  += iou;
                sumDist += dist;
                sumMs   += ms;
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << '\n';
            }
        }

        // Final mean output
        if (n)
        {
            double mIoU  = sumIoU / n;
            double mDist = sumDist / n;
            double mMs   = sumMs / n;

            std::cout << "MEAN  IoU=" << mIoU << "  AvgDist=" << mDist
                      << " px  Time=" << mMs << " ms\n";

            std::ofstream("data/output/complete_results.txt", std::ios::app)
                << "MEAN  IoU=" << mIoU
                << "  AvgDist=" << mDist
                << "  Time=" << mMs << " ms\n";
        }
        return 0;
    }

    // Single image mode
    fs::path imgPath = pos[0];
    // Keep empty if user omitted GT file
    fs::path gtPath;
    if (pos.size() > 1)
        gtPath = pos[1];

    try
    {
        double iou, dist, ms;
        exec(imgPath, gtPath, interactive, iou, dist, ms);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
    return 0;
}



