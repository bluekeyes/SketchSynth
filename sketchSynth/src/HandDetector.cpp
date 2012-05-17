#include "ShapeUtils.h"

#include "HandDetector.h"

using cv::Mat;


//---------------------------------------------------------
HandDetector::HandDetector()
    : fingerThreshold(40 * 40)
    , useCenter(-1, -1)
{
    elem2x2 = Mat::ones(2, 2, CV_8U);
    elem3x3 = Mat::ones(3, 3, CV_8U);

	topFinder.setMinAreaRadius(20);
	topFinder.setMaxAreaRadius(200);
    topFinder.setAutoThreshold(false);
}

//---------------------------------------------------------
bool HandDetector::detect(const cv::Mat &top, const ofPolyline &paper) {
    // Join the hand by dilating small gaps, then get rid of background noise
    // with a lot of erosion
    cv::dilate(top, topFilled, elem2x2, useCenter, 2);
    cv::erode(top, topFilled, elem2x2, useCenter, 6);

    // Fill in the holes in the hand, and the shrink it some for higher
    // accuracy in finger detection
    cv::dilate(topFilled, topFilled, elem3x3, useCenter, 5);
    cv::erode(topFilled, topFilled, elem2x2, useCenter, 3);

    topFinder.findContours(topFilled);

    const size_t n = topFinder.size();
    if (n > 0) {
        float maxArea = -numeric_limits<float>::infinity();
        size_t maxIndex = 0;

        for (size_t i = 0; i < n; i++) {
            float area = topFinder.getContourArea(i);
            if (area > maxArea) {
                maxArea = area;
                maxIndex = i;
            }
        }

        contour = ShapeUtils::filterPolyline(topFinder.getPolyline(maxIndex), 7);
        foundFingerInLast = findFingers(paper);
    } else {
        contour.clear();
        fingers.clear();
        foundFingerInLast = false;
    }
    return foundFingerInLast;
}

//---------------------------------------------------------
bool HandDetector::findFingers(const ofPolyline &paper) {
    float mx = -numeric_limits<float>::infinity();
    float mn = numeric_limits<float>::infinity();

    bool lookForMax = true;
    size_t mxPos = 0;

    fingers.clear();

    ofPoint centroid = ShapeUtils::getCentroid2D(contour);
    for (size_t i = 0; i < contour.size(); i++) {
        float v = ofDistSquared(centroid.x, centroid.y, contour[i].x, contour[i].y);
        if (v > mx) {
            mx = v; mxPos = i;
        } else if (v < mn) {
            mn = v;
        }

        // We can only find a new max after finding a min
        if (lookForMax) {
            if (v < mx - fingerThreshold) {
                // We found a max, so look for a min now
                fingers.push_back(mxPos);
                mn = v;
                lookForMax = false;
            }
        } else {
            if (v > mn + fingerThreshold) {
                // We found a good enough min, so look for another max
                mx = v; mxPos = i;
                lookForMax = true;
            }
        }
    }

    // Find the farthest peak that's inside the paper
    float farthest = -numeric_limits<float>::infinity();
    bool found = false;

    ofPoint bestFinger;
    for (size_t i = 0; i < fingers.size(); i++) {
        float x = contour[fingers[i]].x;
        float y = contour[fingers[i]].y;
        float v = ofDistSquared(centroid.x, centroid.y, x, y);
        if (v > farthest && ShapeUtils::inside(paper, x, y)) {
            farthest = v;
            bestFinger.set(x, y);
            found = true;
        }
    }

    if (found) {
        if (foundFingerInLast) {
            // Better than nothing, but we probably want a freaking Kalman
            // filter, like usual
            fingerPoint = fAlpha * fingerPoint + (1 - fAlpha) * bestFinger;
        } else {
            fingerPoint.set(bestFinger);
        }
    }

    return found;
}

//---------------------------------------------------------
ofPoint HandDetector::getFingerPoint() {
    if (foundFingerInLast) {
        return fingerPoint;
    } else {
        return ofPoint(-5, -5);
    }
}

//---------------------------------------------------------
void HandDetector::draw() {
    contour.draw();
    ofFill();
    for (size_t i = 0; i < fingers.size(); i++) {
        size_t p = fingers[i];
        ofSetColor(0, 255, i * 70);
        ofCircle(contour[p].x, contour[p].y, 10);
    }

    if (foundFingerInLast) {
        ofSetColor(0, 0, 255);
        ofCircle(fingerPoint.x, fingerPoint.y, 10);
    }

    if (contour.size() > 0) {
        ofPoint centroid = ShapeUtils::getCentroid2D(contour);
        ofSetColor(255, 0, 0);
        ofCircle(contour[0].x, contour[0].y, 5);
        ofCircle(centroid.x, centroid.y, 5);
    }
}
    
void HandDetector::drawDetectorInput(float x, float y, float w, float h) {
    ofxCv::drawMat(topFilled, x, y, w, h);
}
