#include "file_io.h"
#include "geometry_utils.h"
#include <fstream>
#include <iostream>

void saveTxt(const fs::path& p, const std::vector<Point2f>& q) {
    std::ofstream f(p);
    f << "(" << (int)q[0].x << "," << (int)q[0].y << "),("
      << (int)q[1].x << "," << (int)q[1].y << "),("
      << (int)q[2].x << "," << (int)q[2].y << "),("
      << (int)q[3].x << "," << (int)q[3].y << ")\n";
}

std::vector<Point2f> readGtFromCoordinatesFile(const fs::path& coordFile, const std::string& imageName) {
    std::vector<Point2f> v;
    std::ifstream f(coordFile);
    
    if(!f.is_open()) {
        std::cerr << "Warning: Cannot open coordinates file: " << coordFile << std::endl;
        return v;
    }
    
    std::string line;
    std::string targetPrefix = imageName + ":";
    
    while(std::getline(f, line)) {
        if(line.find(targetPrefix) == 0) {
            // Found the line for this image
            // Parse format: img_X: "x1 y1"	"x2 y2" 	"x3 y3" 	"x4 y4"
            
            size_t pos = line.find(':');
            if(pos == std::string::npos) continue;
            
            std::string coords = line.substr(pos + 1);
            
            // Extract coordinates from quoted strings
            std::vector<std::pair<float, float>> points;
            size_t start = 0;
            
            while((start = coords.find('"', start)) != std::string::npos) {
                size_t end = coords.find('"', start + 1);
                if(end == std::string::npos) break;
                
                std::string coordPair = coords.substr(start + 1, end - start - 1);
                
                // Parse "x y" format
                std::istringstream iss(coordPair);
                float x, y;
                if(iss >> x >> y) {
                    points.emplace_back(x, y);
                }
                
                start = end + 1;
            }
            
            if(points.size() == 4) {
                for(const auto& p : points) {
                    v.emplace_back(p.first, p.second);
                }
            }
            break;
        }
    }
    
    if(v.size() != 4) {
        std::cerr << "Warning: Expected 4 points for " << imageName << ", got " << v.size() << std::endl;
        if(v.empty()) {
            // Return default points to avoid crash
            v = {Point2f(0,0), Point2f(100,0), Point2f(100,100), Point2f(0,100)};
        }
    }
    
    orderCCW(v);
    return v;
}

std::vector<Point2f> readGt(const fs::path& t) {
    std::vector<Point2f> v;
    std::ifstream f(t);
    
    if(!f.is_open()) {
        std::cerr << "Warning: Cannot open ground truth file: " << t << std::endl;
        return v;
    }
    
    char c;
    while(f >> c) {
        if(c != '(') continue;
        float x, y;
        char comma;
        if(f >> x >> comma >> y) {
            v.emplace_back(x, y);
            f >> c;  // Read closing ')'
        }
    }
    
    if(v.size() != 4) {
        std::cerr << "Warning: Expected 4 points in ground truth, got " << v.size() << std::endl;
        if(v.empty()) {
            // Return default points to avoid crash
            v = {Point2f(0,0), Point2f(100,0), Point2f(100,100), Point2f(0,100)};
        }
    }
    
    orderCCW(v);
    return v;
}
