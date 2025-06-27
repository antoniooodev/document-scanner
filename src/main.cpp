#include "document_detector.h"
#include "file_io.h"
#include "evaluation.h"
#include "visualization.h"
#include "geometry_utils.h"
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <chrono>

namespace fs = std::filesystem;
using cv::Mat;
using cv::Point2f;

// Executes document detection on a single image.

static double exec(const fs::path& imgP, const fs::path& gtP, const fs::path& coordFile = "") {
    std::cout << "Processing: " << imgP.filename() << std::endl;

    // Start timing
    auto start = std::chrono::high_resolution_clock::now();
    
    Mat src = cv::imread(imgP.string());
    if(src.empty()) throw std::runtime_error("imread failed");
    
    double sc = 600.0 / std::max(src.cols, src.rows);
    Mat mini;
    cv::resize(src, mini, {}, sc, sc, cv::INTER_AREA);

    auto quad = detect(mini);
    for(auto& p : quad) clipPt(p, mini.cols, mini.rows);
    
    // Order points as top left, top right, bottom right, bottom left
    auto orderedQuad = orderPoints(quad);

    // Rescale up to original img size (using same factor sc)
    // Higher resolution, downscaled scan is too blurred
    std::vector<cv::Point2f> orderedQuadFullRes;
    for (const auto& pt : orderedQuad) {
        orderedQuadFullRes.emplace_back(pt.x / sc, pt.y / sc);
    }

    // Transform to get top down view
    cv::Mat warped = fourPointTransform(src, orderedQuadFullRes);

    // Handle ground truth
    std::vector<Point2f> gt;
    double iou = -1;
    
    if(!coordFile.empty() && fs::exists(coordFile)) {
        // Use the coordinates.txt file
        std::string imgName = imgP.stem().string();
        auto gt_orig = readGtFromCoordinatesFile(coordFile, imgName);
        
        if(!gt_orig.empty()) {
            // Scale coordinates to match mini image dimensions
            // Original coordinates seem to be in full resolution, so scale them down
            for(const auto& p : gt_orig) {
                float scaled_x = p.x * sc;
                float scaled_y = p.y * sc;
                gt.emplace_back(scaled_x, scaled_y);
            }
            for(auto& p : gt) clipPt(p, mini.cols, mini.rows);
            iou = IoU(quad, gt);
        }
    } else if(!gtP.empty() && fs::exists(gtP)) {
        // Use individual ground truth file
        auto gt_orig = readGt(gtP);
        for(const auto& p : gt_orig) {
            // Scale from (0,0)-(449,599) to mini image dimensions
            float scaled_x = (p.x / 449.0f) * (mini.cols - 1);
            float scaled_y = (p.y / 599.0f) * (mini.rows - 1);
            gt.emplace_back(scaled_x, scaled_y);
        }
        for(auto& p : gt) clipPt(p, mini.cols, mini.rows);
        iou = IoU(quad, gt);
    }
    
    if(gt.empty()) {
        // Create dummy ground truth for visualization
        gt = {Point2f(0,0), Point2f(mini.cols-1,0), Point2f(mini.cols-1,mini.rows-1), Point2f(0,mini.rows-1)};
    }

    // Draw and save visualization
    fs::path outputDir = "output";
    fs::path outputPath = outputDir / (imgP.stem().string() + "_boxes.png");
    drawBoxes(mini, quad, gt, outputPath);
    std::cout << "Saved visualization to: " << outputPath << std::endl;

    // Save warped image (final scan)
    fs::path outputPathWarp = outputDir / (imgP.stem().string() + "_scan.png");
    cv::imwrite(outputPathWarp, warped);
    std::cout << "Saved document to: " << outputPathWarp << std::endl;

    // Stop timing
    auto end = std::chrono::high_resolution_clock::now();
    // Compute and print duration in milliseconds
    double duration_ms = std::chrono::duration<double, std::milli>(end - start).count();

    std::cout << "--> Processing time: " << duration_ms << " ms\n";

    double avgDist = -1;
    // Compute average distance between corners
    if (!gt.empty()) {
        // Re-order ground truth vector clock wise (for correct corner corrispondence)
        gt = orderPoints(gt);
        avgDist = averagePointDistance(quad, gt);
        std::cout << "--> Average distance from ground truth: " << avgDist << " pixels\n";
    }

    // Print IoU
    std::cout << "--> \"" << imgP.filename().string() << "\": IoU=" << iou << '\n';
    std::cout << "\n";

    // Save prediction in current directory
    fs::path predFile = "complete_results.txt";
    // Keep file in append mode to work in a loop
    std::ofstream out(predFile, std::ios::app);

    // Append all measurements for current file
    out << imgP.stem().string() << ": ";
    for (int i = 0; i < 4; ++i) {
        out << "\"" << (int)quad[i].x << " " << (int)quad[i].y << "\"   ";
    }
    out << "\n";
    out << "--> Processing time: " << duration_ms << " ms\n";
    out << "--> Average distance from ground truth: " << avgDist << " pixels\n";
    out << "--> \"" << imgP.filename().string() << ": IoU=" << iou << '\n';
    out << "\n";

    // Return IoU (due to func definition)
    return iou;
}

// Main function - handles command line arguments and dataset processing.
int main(int argc, char** argv) {
    // Clear predictions file at the start (open in truncate mode)
    std::ofstream clearPredFile("complete_results.txt", std::ios::trunc);
    clearPredFile.close();

    if(argc < 2) {
        std::cout << "Usage: ./DocumentScanner img.png [gt.txt] | ./DocumentScanner --dataset DIR\n";
        return 0;
    }
    
    std::string a1 = argv[1];
    if(a1 == "--dataset") {
        fs::path dir = argv[2];
        fs::path coordFile = dir / "../ground_truth/coordinates.txt";
        double sum = 0;
        int n = 0;
        
        for(int k = 1; k <= 10; k++) {
            fs::path img = dir / ("img_" + std::to_string(k) + ".png");
            if(!fs::exists(img)) {
                // Try .jpg extension
                img = dir / ("img_" + std::to_string(k) + ".jpg");
            }
            if(!fs::exists(img)) continue;
            
            try {
                double i = exec(img, "", coordFile);
                if(i >= 0) {
                    sum += i;
                    n++;
                }
            } catch(const std::exception& e) {
                std::cerr << "Err " << img.filename() << ": " << e.what() << "\n";
            }
        }
        
        if(n) std::cout << "Mean IoU=" << sum / n << "\n";
    } else {
        fs::path img = a1;
        fs::path gt = (argc > 2 ? fs::path(argv[2]) : fs::path());
        fs::path coordFile = "../data/ground_truth/coordinates.txt";
        
        try {
            exec(img, gt, coordFile);
        } catch(const std::exception& e) {
            std::cerr << e.what() << "\n";
        }
    }
    
    return 0;
}