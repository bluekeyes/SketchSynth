#include <math.h>

#include "ControlManager.h"

using cv::Mat;
using cv::RotatedRect;

const ofColor ControlManager::accent1 = ofColor(100, 0, 57);
const ofColor ControlManager::accent2 = ofColor(45, 0, 180);

//---------------------------------------------------------
ControlManager::~ControlManager() {
    // Reset will delete all existing control elements
    reset();
}

//---------------------------------------------------------
void ControlManager::setup() {
	finder.setMinAreaRadius(20);
	finder.setMaxAreaRadius(120);
    // Don't threshold, will run edge detection instead
    finder.setAutoThreshold(false);

    sender.setup();

    buttons.clear();
    sliders.clear();
    switches.clear();
    controls.clear();

    colors.push_back(ofColor(140, 0, 100));
    colors.push_back(ofColor(87, 0, 210));
}

//---------------------------------------------------------
void ControlManager::reset() {
    const size_t n = controls.size();
    for (size_t i = 0; i < n; i++) {
        if (controls[i] != NULL) {
            delete controls[i];
        }
    }

    buttons.clear();
    sliders.clear();
    switches.clear();
    controls.clear();
}

//---------------------------------------------------------
void ControlManager::detect(Mat img) {
    // Get the image in the right format and run edge detection
    ofxCv::convertColor(img, grayImg, CV_RGB2GRAY);
    cv::Canny(grayImg, edgesInput, 160, 180, 3);
    cv::dilate(edgesInput, edgesInput, Mat::ones(2, 2, CV_8U), cv::Point(-1, -1), 7);
    cv::erode(edgesInput, edgesInput, Mat::ones(2, 2, CV_8U), cv::Point(-1, -1), 5);

    const cv::Rect &roi = cv::Rect(BORDER, BORDER, edgesInput.cols - 2*BORDER, edgesInput.rows - 2*BORDER);
    edgesInput(roi).copyTo(edges);

    // Find contours in the edge detected image
    finder.findContours(edges);

    size_t n = finder.size();

    for (size_t i = 0; i < n; i++) {
        if (isButton(i)) {
            float radius;
            cv::Point center = finder.getMinEnclosingCircle(i, radius);
            Button *b = new Button(center.x, center.y, radius, sender);

            buttons.push_back(b);
            controls.push_back(static_cast<Control*>(b));
        } else if (isSlider(i)) {
            cv::RotatedRect rect = finder.getMinAreaRect(i);
            Slider *s = new Slider(rect, sender);

            sliders.push_back(s);
            controls.push_back(static_cast<Control*>(s));
        } else if (isSwitch(i)) {
            cv::RotatedRect rect = finder.getMinAreaRect(i);
            Switch *s = new Switch(rect, sender);

            switches.push_back(s);
            controls.push_back(static_cast<Control*>(s));
        }
    }
    
    assignControls();

    sender.sendControlCount(MOMENTARY, buttons.size());
    sender.sendControlCount(CONTINUOUS, sliders.size());
    sender.sendControlCount(TOGGLE, switches.size());
}

//---------------------------------------------------------
void ControlManager::assignControls() {
    for (size_t i = 0; i < buttons.size(); i++) {
        buttons[i]->setId((int) i);
        buttons[i]->setColor(colors[i % colors.size()]);
    }
    
    for (size_t i = 0; i < sliders.size(); i++) {
        sliders[i]->setId((int) i);
        sliders[i]->setColor(colors[i % colors.size()]);
    }
    
    for (size_t i = 0; i < switches.size(); i++) {
        switches[i]->setId((int) i);
        switches[i]->setColor(colors[i % colors.size()]);
    }
}

//---------------------------------------------------------
void ControlManager::processInteraction(const ofPoint &point) {
    lastInputPoint = point;
    for (size_t i = 0; i < controls.size(); i++) {
        if (controls[i]->onInteraction(point)) {
            // Stop after the first control to handle this input
            break;
        }
    }
}

//---------------------------------------------------------
void ControlManager::drawControls() {
    size_t n = controls.size();
    for (size_t i = 0; i < n; i++) {
        controls[i]->draw();
    }

    ofSetColor(accent1);
    ofFill();
    ofCircle(lastInputPoint.x, lastInputPoint.y, 5);
}

//---------------------------------------------------------
vector< pair<string, size_t> > ControlManager::listControls() {
    vector< pair<string, size_t> > list;

    list.push_back(pair<string, size_t>("Buttons", buttons.size()));
    list.push_back(pair<string, size_t>("Sliders", sliders.size()));
    list.push_back(pair<string, size_t>("Switches", switches.size()));

    return list;
}

//---------------------------------------------------------
OscSender& ControlManager::getSender() {
    return sender;
}

//---------------------------------------------------------
void ControlManager::drawDetectorInput(float x, float y, float w, float h) {
    ofxCv::drawMat(edges, x, y, w, h);
}

//---------------------------------------------------------
bool ControlManager::isButton(size_t i) {
    float radius;
    finder.getMinEnclosingCircle(i, radius);

    float circleArea = PI * radius * radius;
    double contourArea = finder.getContourArea(i);

    return abs(1.0 - circleArea / contourArea) < 0.3;
}

//---------------------------------------------------------
bool ControlManager::isSlider(size_t i) {
    float radius;
    ofVec2f center = ofxCv::toOf(finder.getMinEnclosingCircle(i, radius));

    float circleArea = PI * radius * radius;
    double contourArea = finder.getContourArea(i);

    // TODO Possibly consider width/height ratio as well
    return circleArea / contourArea > 12;
}

//---------------------------------------------------------
bool ControlManager::isSwitch(size_t i) {
    RotatedRect rr = finder.getMinAreaRect(i);

    float rectArea = rr.size.width * rr.size.height;
    double contourArea = finder.getContourArea(i);

    return abs(1.0 - rectArea / contourArea) < 0.25
        && abs(1.0 - (float) rr.size.width / rr.size.height) > 0.1;
}
