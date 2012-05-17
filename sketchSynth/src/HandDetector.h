#pragma once

#include "ofMain.h"
#include "ofxCv.h"

#include "PaperDetector.h"

class HandDetector {
public:
    HandDetector();

    void draw();
    void drawDetectorInput(float x, float y, float w, float h);
    
    template <class T, class S>
    bool detect(const T &top, const S &side, const ofPolyline &paper) {
        return detect(ofxCv::toCv(top), ofxCv::toCv(side), paper);
    }
    bool detect(const cv::Mat &top, const ofPolyline &paper);
    
    ofPoint getFingerPoint();

private:
    bool findFingers(const ofPolyline &paper);

    ofxCv::ContourFinder topFinder;
    ofxCv::ContourFinder sideFinder;
    
    cv::Mat topFilled;
    ofPolyline contour;

    vector<size_t> fingers;
    ofPoint fingerPoint;
    bool foundFingerInLast;

    float fingerThreshold;
    static const float fAlpha = 0.2;

    cv::Mat elem2x2;
    cv::Mat elem3x3;
    const cv::Point useCenter;
};
