#pragma once

#include <algorithm>

#include "ofMain.h"
#include "ofxCv.h"

namespace ShapeUtils {
    static const float DEFAULT_ANGLE = 2.5;

    bool isRectangle(const ofPolyline &poly, float angle = DEFAULT_ANGLE);
    template <class T>
    bool isRectangle(const T &poly, float angle = DEFAULT_ANGLE) {
        return isRectangle(ofxCv::toOf(poly));
    }

    float polylineArea(const ofPolyline &poly);

    ofPoint getCentroid2D(const ofPolyline &poly);

    bool inside(const ofPolyline &polyline, float x, float y);
    
    ofPolyline filterPolyline(const ofPolyline &poly, const int k);

    template <class T>
    void orderQuadForTransform(vector<T> &pts) {
        ofVec2f s01 = ofVec2f(pts[1].x - pts[0].x, pts[1].y - pts[0].y);
        ofVec2f s12 = ofVec2f(pts[2].x - pts[1].x, pts[2].y - pts[1].y);

        ofVec3f cross = ofVec3f(s01.x, s01.y, 0).getCrossed(ofVec3f(s12.x, s12.y, 0));
        // CCW winding has a negative cross product
        if (cross.z < 0) {
            // Transform to CW winding
            std::reverse(pts.begin(), pts.end());
            // Recompute the side vectors if we change the winding
            s01 = ofVec2f(pts[1].x - pts[0].x, pts[1].y - pts[0].y);
            s12 = ofVec2f(pts[2].x - pts[1].x, pts[2].y - pts[1].y);
        }
        
        int rotate = 0;
        // If the 1 --> 2 side is the width, we need to rotate by 1
        if (s01.squareLength() < s12.squareLength()) {
            rotate += 1;
        }

        ofVec2f plusX = ofVec2f(1, 0);
        if (s01.dot(plusX) < 0) {
            rotate += 2;
        }

        std::rotate(pts.begin(), pts.begin() + rotate, pts.end());
    }

    void applyTransform(const cv::Mat &mtx);

    ofPoint warpPoint(const ofPoint &point, cv::Mat &mtx);
};
