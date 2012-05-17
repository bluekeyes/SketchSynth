#include <math.h>

#include "ShapeUtils.h"

bool ShapeUtils::isRectangle(const ofPolyline &poly, float angle) {
    if (poly.size() != 4) {
       return false;
    }

    float delta = cos((90 - angle) * PI / 180);

    // Make sure it's a rectangle
    ofVec2f top    = ofVec2f(poly[1].x - poly[0].x, poly[1].y - poly[0].y);
    ofVec2f right  = ofVec2f(poly[2].x - poly[1].x, poly[2].y - poly[1].y);
    ofVec2f bottom = ofVec2f(poly[3].x - poly[2].x, poly[3].y - poly[2].y);
    ofVec2f left   = ofVec2f(poly[0].x - poly[3].x, poly[0].y - poly[3].y);

    top.normalize();
    right.normalize();
    bottom.normalize();
    left.normalize();

    bool isRect = abs(top.dot(right))    < delta
               && abs(right.dot(bottom)) < delta
               && abs(bottom.dot(left))  < delta
               && abs(left.dot(top))     < delta;

    return isRect;
}

// Backported from oF-dev branch on github
float ShapeUtils::polylineArea(const ofPolyline &poly) {
    if (poly.size() < 2) return 0;

    float area = 0;
    for (int i = 0; i < (int) poly.size() - 1; i++) {
        area += poly[i].x * poly[i+1].y - poly[i+1].x * poly[i].y;
    }
    area += poly[poly.size()-1].x * poly[0].y - poly[0].x * poly[poly.size()-1].y;
    return 0.5 * area;
}

// Backported from oF-dev branch on github
ofPoint ShapeUtils::getCentroid2D(const ofPolyline &poly) {
    ofPoint centroid;
    for(int i=0;i<(int)poly.size()-1;i++){
        centroid.x += (poly[i].x + poly[i+1].x) * (poly[i].x*poly[i+1].y - poly[i+1].x*poly[i].y);
        centroid.y += (poly[i].y + poly[i+1].y) * (poly[i].x*poly[i+1].y - poly[i+1].x*poly[i].y);
    }
    centroid.x += (poly[poly.size()-1].x + poly[0].x) * (poly[poly.size()-1].x*poly[0].y - poly[0].x*poly[poly.size()-1].y);
    centroid.y += (poly[poly.size()-1].y + poly[0].y) * (poly[poly.size()-1].x*poly[0].y - poly[0].x*poly[poly.size()-1].y);

    float area = ShapeUtils::polylineArea(poly);
    centroid.x /= (6*area);
    centroid.y /= (6*area);
    return centroid;
}

// Backported from oF-dev branch on github
bool ShapeUtils::inside(const ofPolyline &polyline, float x, float y) {
    int counter = 0;
    int i;
    double xinters;
    ofPoint p1,p2;

    int N = polyline.size();
    if (N == 0) {
        return false;
    }

    p1 = polyline[0];
    for (i=1;i<=N;i++) {
        p2 = polyline[i % N];
        if (y > MIN(p1.y,p2.y)) {
            if (y <= MAX(p1.y,p2.y)) {
                if (x <= MAX(p1.x,p2.x)) {
                    if (p1.y != p2.y) {
                        xinters = (y-p1.y)*(p2.x-p1.x)/(p2.y-p1.y)+p1.x;
                        if (p1.x == p2.x || x <= xinters)
                            counter++;
                    }
                }
            }
        }
        p1 = p2;
    }

    if (counter % 2 == 0) return false;
    else return true;
}


ofPolyline ShapeUtils::filterPolyline(const ofPolyline &poly, const int k) {
    const int nPoints = poly.size();
    ofPolyline filtered;

    ofPoint avgPoint;
    for (int i = 0; i < nPoints; i++ ) {
        avgPoint.set(0, 0);
        for (int j = i - k; j < i + k + 1; j++) {
            int pos = j;
            if (pos >= nPoints)
                pos = pos % nPoints;
            if (pos < 0)
                pos = nPoints + pos;

            avgPoint += poly[pos];
        }
        filtered.addVertex(avgPoint / (2 * k + 1));
    }

    return filtered;
}

void ShapeUtils::applyTransform(const cv::Mat &mtx) {
    // Convert incoming matrix to float for OpenGL
    cv::Mat mtxf;
    if (mtx.depth() != CV_32F) {
        mtx.convertTo(mtxf, CV_32F);
    } else {
        mtxf = mtx;
    }

    cv::Mat mtx3D = cv::Mat::eye(4, 4, CV_32FC1);
    mtx3D.at<float>(0, 0) = mtxf.at<float>(0, 0);
    mtx3D.at<float>(0, 1) = mtxf.at<float>(0, 1);
    mtx3D.at<float>(0, 3) = mtxf.at<float>(0, 2);
    mtx3D.at<float>(1, 0) = mtxf.at<float>(1, 0);
    mtx3D.at<float>(1, 1) = mtxf.at<float>(1, 1);
    mtx3D.at<float>(1, 3) = mtxf.at<float>(1, 2);
    mtx3D.at<float>(3, 0) = mtxf.at<float>(2, 0);
    mtx3D.at<float>(3, 1) = mtxf.at<float>(2, 1);

    glMultTransposeMatrixf(mtx3D.ptr<float>(0));
}

ofPoint ShapeUtils::warpPoint(const ofPoint &point, cv::Mat &mtx) {
    float pv[2] = {point.x, point.y};
    cv::Mat srcPoint = cv::Mat(1, 1, CV_32FC2, pv);
    cv::Mat dstPoint;
    cv::perspectiveTransform(srcPoint, dstPoint, mtx);

    dstPoint = dstPoint.reshape(1);
    return ofPoint(dstPoint.at<float>(0), dstPoint.at<float>(1));
}
