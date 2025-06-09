#include "document_detector.h"
#include "file_io.h"
#include "evaluation.h"
#include "visualization.h"
#include "geometry_utils.h"
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;
using cv::Mat;
using cv::Point2f;

/**
 * Executes document detection on a single image.
 */
static double exec(const fs::path& imgP, const fs::path& gtP, const fs::path& jsonDir, 
                   const fs::path& coordFile = "") {
    std::cout << "Processing: " << imgP.filename() << std::endl;
    
    Mat src = cv::imread(imgP.string());
    if(src.empty()) throw std::runtime_error("imread failed");
    
    double sc = 600.0 / std::max(src.cols, src.rows);
    Mat mini;
    cv::resize(src, mini, {}, sc, sc, cv::INTER_AREA);

    auto quad = detect(mini);
    for(auto& p : quad) clipPt(p, mini.cols, mini.rows);
    
    // Save prediction in current directory
    fs::path predFile = imgP.filename();
    predFile = predFile.stem().string() + "_predc.txt";
    saveTxt(predFile, quad);
    std::cout << "Saved predictions to: " << predFile << std::endl;

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

    // Save JSON results
    fs::create_directories(jsonDir);
    cv::FileStorage js((jsonDir / (imgP.stem().string() + ".json")).string(),
                       cv::FileStorage::WRITE | cv::FileStorage::FORMAT_JSON);
    js << "image" << imgP.filename().string() 
       << "size" << "[" << mini.cols << mini.rows << "]"
       << "quad" << quad 
       << "gt_quad" << gt 
       << "iou" << iou;
    js.release();

    // Draw and save visualization
    fs::path outputDir = "output";
    fs::path outputPath = outputDir / (imgP.stem().string() + "_boxes.png");
    drawBoxes(mini, quad, gt, outputPath);
    std::cout << "Saved visualization to: " << outputPath << std::endl;

    std::cout << '"' << imgP.filename().string() << "\": IoU=" << iou << '\n';
    return iou;
}

/**
 * Main function - handles command line arguments and dataset processing.
 */
int main(int argc, char** argv) {
    if(argc < 2) {
        std::cout << "Usage: ./DocumentScanner img.png [gt.txt] | ./DocumentScanner --dataset DIR\n";
        return 0;
    }
    
    std::string a1 = argv[1];
    if(a1 == "--dataset") {
        fs::path dir = argv[2], json = dir / "json";
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
                double i = exec(img, "", json, coordFile);
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
        fs::path jsonDir = "json";
        fs::path coordFile = "../data/ground_truth/coordinates.txt";
        
        try {
            exec(img, gt, jsonDir, coordFile);
        } catch(const std::exception& e) {
            std::cerr << e.what() << "\n";
        }
    }
    
    return 0;
}
