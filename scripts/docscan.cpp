/*
 * docscan.cpp  –  v9  (whiteness + strong border kill + minAreaRect per contour) - OPTIMIZED
 * Build: g++ -std=c++17 docscan.cpp -o docscan `pkg-config --cflags --libs opencv4`
 * 
 * OPTIMIZED PARAMETERS from tuning - IoU improved from 0.765692 to 0.839870
 */
#include <opencv2/opencv.hpp>
#ifdef HAVE_OPENCV_XIMGPROC
#include <opencv2/ximgproc.hpp>
#endif
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <numeric>
namespace fs = std::filesystem;
using cv::Mat; using cv::Point2f; using cv::Point;

// ---------- utils ----------------------------------------------------------
static void orderCCW(std::vector<Point2f>&q){
    Point2f c(0,0); for(auto&p:q) c+=p; c*=0.25f;
    std::sort(q.begin(),q.end(),[&](auto&a,auto&b){
        return std::atan2(a.y-c.y,a.x-c.x) < std::atan2(b.y-c.y,b.x-c.x);});
    size_t tl=0; for(size_t i=1;i<4;i++)
        if(q[i].y<q[tl].y||(q[i].y==q[tl].y&&q[i].x<q[tl].x)) tl=i;
    std::rotate(q.begin(),q.begin()+tl,q.end());
}
static bool crossSelf(const std::vector<Point2f>&q){
    auto z=[](Point2f a,Point2f b,Point2f c){return (b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x);};
    return z(q[0],q[1],q[2])*z(q[0],q[1],q[3])<0 &&
           z(q[2],q[3],q[0])*z(q[2],q[3],q[1])<0;
}
static void clipPt(Point2f& p,int W,int H){
    p.x=std::clamp(p.x,0.f,(float)(W-1));
    p.y=std::clamp(p.y,0.f,(float)(H-1));
}
static void saveTxt(const fs::path&p,const std::vector<Point2f>&q){
    std::ofstream f(p); f<<"("<<(int)q[0].x<<","<<(int)q[0].y<<"),("
     <<(int)q[1].x<<","<<(int)q[1].y<<"),("
     <<(int)q[2].x<<","<<(int)q[2].y<<"),("
     <<(int)q[3].x<<","<<(int)q[3].y<<")\n";
}
static std::vector<Point2f> readGt(const fs::path&t){
    std::vector<Point2f> v; std::ifstream f(t); char c;
    while(f>>c){ if(c!='(') continue; float x,y; char comma; f>>x>>comma>>y; v.emplace_back(x,y); f>>c;}
    orderCCW(v); return v;
}
static double IoU(std::vector<Point2f>a,std::vector<Point2f>b){
    if(a.size()!=4||b.size()!=4) return -1;
    std::vector<cv::Point> A,B; for(auto&p:a)A.emplace_back(p); for(auto&p:b)B.emplace_back(p);
    double a1=fabs(cv::contourArea(A)), a2=fabs(cv::contourArea(B));
    cv::Mat inter; bool ok=cv::intersectConvexConvex(A,B,inter);
    double ai=(ok&& !inter.empty())? fabs(cv::contourArea(inter)):0;
    return (a1+a2-ai)>1e-5? ai/(a1+a2-ai):0;
}

// ---------- draw boxes on image --------------------------------------------
static void drawBoxes(const Mat& img, const std::vector<Point2f>& detected, const std::vector<Point2f>& gt, const fs::path& outputPath) {
    Mat result = img.clone();
    
    // Draw ground truth box in green
    if (gt.size() == 4) {
        std::vector<cv::Point> gtPoly;
        for (const auto& p : gt) {
            gtPoly.push_back(cv::Point((int)p.x, (int)p.y));
        }
        cv::polylines(result, gtPoly, true, cv::Scalar(0, 255, 0), 3); // Green
        
        // Add labels
        cv::putText(result, "GT", cv::Point((int)gt[0].x, (int)gt[0].y - 10), 
                   cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);
    }
    
    // Draw detected box in red
    if (detected.size() == 4) {
        std::vector<cv::Point> detPoly;
        for (const auto& p : detected) {
            detPoly.push_back(cv::Point((int)p.x, (int)p.y));
        }
        cv::polylines(result, detPoly, true, cv::Scalar(0, 0, 255), 3); // Red
        
        // Add labels
        cv::putText(result, "DET", cv::Point((int)detected[0].x, (int)detected[0].y - 10), 
                   cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 255), 2);
    }
    
    // Save the result
    fs::create_directories(outputPath.parent_path());
    cv::imwrite(outputPath.string(), result);
}

// ---------- main detector --------------------------------------------------
static std::vector<Point2f> detect(const Mat& img)
{
    int W=img.cols, H=img.rows; double Aimg=img.total();

    // preprocess - OPTIMIZED PARAMETERS
    Mat gray; cv::cvtColor(img,gray,cv::COLOR_BGR2GRAY);
    Mat eq; cv::createCLAHE(3.875,cv::Size(9,9))->apply(gray,eq);  // optimized: clip=3.875, tile=9
    Mat sx,sy,mag; cv::Sobel(eq,sx,CV_32F,1,0); cv::Sobel(eq,sy,CV_32F,0,1);
    cv::magnitude(sx,sy,mag); double mx; cv::minMaxLoc(mag,0,&mx);
    mag.convertTo(mag,CV_8U,255.0/(mx+1e-3));
    cv::threshold(mag,mag,0,255,cv::THRESH_BINARY|cv::THRESH_OTSU);
    cv::morphologyEx(mag,mag,cv::MORPH_CLOSE,Mat(),{-1,-1},4);  // optimized: iterations=4

    // mediana |∇|
    std::vector<uchar> v; v.reserve(mag.total());
    for(int r=0;r<mag.rows;r++) for(int c=0;c<mag.cols;c++) if(mag.at<uchar>(r,c)) v.push_back(mag.at<uchar>(r,c));
    double medGrad = v.empty()? 0 : v[v.size()/2];

    // contours
    std::vector<std::vector<cv::Point>> C;
    cv::findContours(mag,C,cv::RETR_LIST,cv::CHAIN_APPROX_SIMPLE);

    auto edgeMean=[&](const std::vector<Point2f>&q){
        Mat sob; cv::Sobel(eq,sob,CV_32F,1,1);
        double s=0; int n=0;
        for(int i=0;i<4;i++){
            cv::LineIterator it(sob,q[i],q[(i+1)&3],8);
            for(int j=0;j<it.count;++j,++it){ s+=std::abs((*it)[0]); ++n; }
        }
        return n? s/n:0;
    };
    auto borderFrac=[&](const std::vector<Point2f>&q){
        double touch=0, per=0;
        for(int i=0;i<4;i++){
            auto a=q[i],b=q[(i+1)&3]; double len=cv::norm(a-b); per+=len;
            if((a.x<2&&b.x<2)||(a.x>W-3&&b.x>W-3)||(a.y<2&&b.y<2)||(a.y>H-3&&b.y>H-3))
                touch+=len;
        }
        return touch/per;
    };
    auto whiteness=[&](const std::vector<Point2f>&q){
        std::vector<cv::Point> poly; for(auto&p:q) poly.emplace_back(p);
        Mat mask=Mat::zeros(gray.size(),CV_8U);
        cv::fillConvexPoly(mask,poly,255);
        cv::Scalar mDoc=cv::mean(gray,mask);
        cv::Scalar mBg=cv::mean(gray,255-mask);
        double w = (mBg[0]>1)? mDoc[0]/mBg[0] : 1;
        if(w<1) w=1/w;                   // distanza da 1
        return std::clamp((w-1)/0.5,0.0,1.0); // 0 se simile, 1 se ≥1.5x
    };

    struct Cand{std::vector<Point2f> q; double sc;};
    std::vector<Cand> list;

    auto evalQuad=[&](std::vector<Point2f> q){
        if(crossSelf(q)) return;
        double A=fabs(cv::contourArea(q)); if(A < 0.029*Aimg) return;  // optimized: area_threshold=0.029
        double bF=borderFrac(q); if(bF>0.476) return;          // optimized: border_kill=0.476
        double areaFit = 1-std::abs(A-0.458*Aimg)/(0.458*Aimg);  // optimized: target_area=0.458

        double ar = std::max(cv::norm(q[0]-q[1]),cv::norm(q[1]-q[2])) /
                    std::min(cv::norm(q[0]-q[1]),cv::norm(q[1]-q[2]));
        double ARfit = 1-std::min(std::abs(ar-1.414)/1.0,1.0);

        double gradFit = medGrad>1? std::clamp(edgeMean(q)/(edgeMean(q)+medGrad),0.0,1.0):0.5;
        double wFit = whiteness(q);

        // optimized weights: [0.329, 0.266, 0.208, 0.197]
        double score = 0.329*areaFit + 0.266*wFit + 0.208*gradFit + 0.197*ARfit;
        list.push_back({q,score});
    };

    // 1. polyDP =4
    for(auto &cont:C){
        std::vector<cv::Point> ap;
        cv::approxPolyDP(cont,ap,0.005*cv::arcLength(cont,true),true);  // optimized: epsilon=0.005
        if(ap.size()==4 && cv::isContourConvex(ap)){
            std::vector<Point2f> q(ap.begin(),ap.end()); orderCCW(q); evalQuad(q);
        }
    }
    // 2. minAreaRect di ogni contorno
    for(auto &cont:C){
        cv::RotatedRect rr=cv::minAreaRect(cont); Point2f r[4]; rr.points(r);
        std::vector<Point2f> q(r,r+4); orderCCW(q); evalQuad(q);
    }
#ifdef HAVE_OPENCV_XIMGPROC
    // 3. LSD + RANSAC (facoltativo)
    {
        Mat edges; cv::Canny(eq,edges,50,150);
        auto fld=cv::ximgproc::createFastLineDetector(); std::vector<cv::Vec4f> segs; fld->detect(edges,segs);
        if(segs.size()>=4){
            // prendi 4 segmenti più lunghi
            std::sort(segs.begin(),segs.end(),[](auto&a,auto&b){
                double la=cv::norm(Point2f(a[0],a[1])-Point2f(a[2],a[3]));
                double lb=cv::norm(Point2f(b[0],b[1])-Point2f(b[2],b[3]));
                return la>lb;});
            std::vector<Point2f> pts;
            for(int i=0;i<4;i++){ pts.emplace_back(segs[i][0],segs[i][1]); pts.emplace_back(segs[i][2],segs[i][3]);}
            cv::RotatedRect rr=cv::minAreaRect(pts); Point2f r[4]; rr.points(r);
            std::vector<Point2f> q(r,r+4); orderCCW(q); evalQuad(q);
        }
    }
#endif
    // scelta miglior score - OPTIMIZED THRESHOLD
    std::vector<Point2f> best;
    if(!list.empty()){
        auto top=*std::max_element(list.begin(),list.end(),[](auto&a,auto&b){return a.sc<b.sc;});
        if(top.sc>=0.3) best=top.q;  // optimized: min_score=0.3
    }

    if(best.empty()){
        // minAreaRect del contorno maggiore
        auto &big=*std::max_element(C.begin(),C.end(),[](auto&a,auto&b){
            return fabs(cv::contourArea(a))<fabs(cv::contourArea(b));});
        cv::RotatedRect rr=cv::minAreaRect(big); Point2f r[4]; rr.points(r);
        best.assign(r,r+4); orderCCW(best);
    }

    // refine ±2 px se border safe (mantiene parametri originali per il refinement)
    if(borderFrac(best)<0.2){
        for(int i=0;i<4;i++){
            auto d=best[(i+1)&3]-best[i]; double L=cv::norm(d);
            if(L>0){ cv::Point2f n(d.y/L,-d.x/L); best[i]-=2*n; best[(i+1)&3]+=2*n; }
        }
    }
    for(auto&p:best) clipPt(p,W,H);
    return best;
}
// ---------- run single -----------------------------------------------------
static double exec(const fs::path& imgP,const fs::path& gtP,const fs::path& jsonDir)
{
    Mat src=cv::imread(imgP.string()); if(src.empty()) throw std::runtime_error("imread");
    double sc=600.0/std::max(src.cols,src.rows); Mat mini; cv::resize(src,mini,{},sc,sc,cv::INTER_AREA);

    auto quad=detect(mini);
    for(auto&p:quad) clipPt(p,mini.cols,mini.rows);
    { fs::path p=imgP; p.replace_extension("_predc.txt"); saveTxt(p,quad); }

    // Read ground truth and scale coordinates to match mini image dimensions
    auto gt_orig=readGt(gtP);
    std::vector<Point2f> gt;
    for(const auto& p : gt_orig) {
        // Scale from (0,0)-(449,599) to mini image dimensions
        float scaled_x = (p.x / 449.0f) * (mini.cols - 1);
        float scaled_y = (p.y / 599.0f) * (mini.rows - 1);
        gt.emplace_back(scaled_x, scaled_y);
    }
    for(auto&p:gt) clipPt(p,mini.cols,mini.rows);
    
    double iou=IoU(quad,gt);

    fs::create_directories(jsonDir);
    cv::FileStorage js((jsonDir/(imgP.stem().string()+".json")).string(),
                       cv::FileStorage::WRITE|cv::FileStorage::FORMAT_JSON);
    js<<"image"<<imgP.filename().string()<<"size"<<"["<<mini.cols<<mini.rows<<"]"
       <<"quad"<<quad<<"gt_quad"<<gt<<"iou"<<iou; js.release();

    // Draw and save visualization
    fs::path outputDir = "output";
    fs::path outputPath = outputDir / (imgP.stem().string() + "_boxes.png");
    drawBoxes(mini, quad, gt, outputPath);

    std::cout<<'"'<<imgP.filename().string()<<"\": IoU="<<iou<<'\n';
    return iou;
}
// ---------- main -----------------------------------------------------------
int main(int argc,char**argv)
{
    if(argc<2){ std::cout<<"Usage: ./docscan img.png gt.txt | ./docscan --dataset DIR\n"; return 0; }
    std::string a1=argv[1];
    if(a1=="--dataset"){
        fs::path dir=argv[2], json=dir/"json"; double sum=0; int n=0;
        for(int k=1;k<=10;k++){
            fs::path img=dir/("img_"+std::to_string(k)+".png");
            fs::path gt =dir/("img_"+std::to_string(k)+"_optc.txt");
            if(!fs::exists(img)||!fs::exists(gt)) continue;
            try{ double i=exec(img,gt,json); if(i>=0){sum+=i;n++;}}
            catch(const std::exception&e){ std::cerr<<"Err "<<img.filename()<<": "<<e.what()<<"\n"; }
        }
        if(n) std::cout<<"Mean IoU="<<sum/n<<"\n";
    }else{
        fs::path img=a1, gt=(argc>2? fs::path(argv[2]):fs::path()), jsonDir="json";
        try{ exec(img,gt,jsonDir);}catch(const std::exception&e){ std::cerr<<e.what()<<"\n";}
    }
    return 0;
}
