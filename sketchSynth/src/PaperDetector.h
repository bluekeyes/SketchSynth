#pragma once

#include "ofMain.h"
#include "ofxCv.h"

#include "ShapeUtils.h"

class PaperDetector {
public:
    void setup();
    void draw();
    
    template <class T>
    bool detect(T &img) {
        return detect(ofxCv::toCv(img));
    }
    bool detect(cv::Mat img);
    
    template <class S, class D>
    void unwarp(S &src, D &dst) {
        vector<cv::Point2f> warpPoints;
        copy(paper.begin(), paper.end(), back_inserter(warpPoints));
        ShapeUtils::orderQuadForTransform(warpPoints);
        ofxCv::unwarpPerspective(src, dst, warpPoints);
    }

    template <class D>
    void unwarp(D &dst) {
        unwarp(paperImage, dst);
    }

    cv::Mat getTransformation(int outWidth, int outHeight);

    ofPoint unwarpPoint(const ofPoint &point, int outWidth, int outHeight);

    ofPolyline getPaper();

private:
    ofxCv::ContourFinder finder;

    cv::Mat paperImage;
    vector<cv::Point> paper;
};
