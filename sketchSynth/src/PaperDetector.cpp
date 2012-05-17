#include "ShapeUtils.h"

#include "PaperDetector.h"

using ofxCv::toOf;
using cv::Mat;
using cv::Point2f;

void PaperDetector::setup() {
	finder.setMinAreaRadius(50);
	finder.setMaxAreaRadius(200);
	finder.setThreshold(120);
}

void PaperDetector::draw() {
    if (paper.size() == 4) {
        ofNoFill();
        ofSetColor(ofxCv::magentaPrint);
        ofxCv::toOf(paper).draw();
    }
}

bool PaperDetector::detect(cv::Mat img) {
    finder.findContours(img);

    vector<cv::Point> maxQuad;
    float maxArea = -numeric_limits<float>::infinity();

    const size_t n = finder.size();
    for(size_t i = 0; i < n; i++) {
        vector<cv::Point> quad = finder.getFitQuad(i);
        ofPolyline contour = toOf(quad);

        float area = ShapeUtils::polylineArea(contour);
        if (area > maxArea) {
            maxArea = area;
            maxQuad = quad;
        }
    }

    // Make sure it's a rectangle
    bool isRect = ShapeUtils::isRectangle(maxQuad);
    if (isRect) {
        paperImage = img;
        paper = maxQuad;
    }
    return isRect;
}

Mat PaperDetector::getTransformation(int outWidth, int outHeight) {
    vector<Point2f> warpPoints;
    copy(paper.begin(), paper.end(), back_inserter(warpPoints));
    ShapeUtils::orderQuadForTransform(warpPoints);

    vector<Point2f> dstPoints(4);
    dstPoints[0] = Point2f(0, 0);
    dstPoints[1] = Point2f(outWidth, 0);
    dstPoints[2] = Point2f(outWidth, outHeight);
    dstPoints[3] = Point2f(0, outHeight);

    return cv::getPerspectiveTransform(&dstPoints[0], &warpPoints[0]);
}

ofPoint PaperDetector::unwarpPoint(const ofPoint &point, int outWidth, int outHeight) {
    vector<Point2f> warpPoints;
    copy(paper.begin(), paper.end(), back_inserter(warpPoints));
    ShapeUtils::orderQuadForTransform(warpPoints);

    vector<Point2f> dstPoints(4);
    dstPoints[0] = Point2f(0, 0);
    dstPoints[1] = Point2f(outWidth, 0);
    dstPoints[2] = Point2f(outWidth, outHeight);
    dstPoints[3] = Point2f(0, outHeight);

    Mat transform = cv::getPerspectiveTransform(&warpPoints[0], &dstPoints[0]);
    float pv[2] = {point.x, point.y};
    Mat srcPoint = Mat(1, 1, CV_32FC2, pv);
    Mat dstPoint;
    cv::perspectiveTransform(srcPoint, dstPoint, transform);

    dstPoint = dstPoint.reshape(1);
    return ofPoint(dstPoint.at<float>(0), dstPoint.at<float>(1));
}

ofPolyline PaperDetector::getPaper() {
    return ofxCv::toOf(paper);
}
