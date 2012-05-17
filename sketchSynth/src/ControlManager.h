#pragma once

#include <vector>

#include "ofMain.h"
#include "ofxCv.h"

#include "Control.h"
#include "OscSender.h"

class ControlManager {
public:
    ~ControlManager();

    void setup();
    void reset();

    void drawControls();
    void drawDetectorInput(float x, float y, float w, float h);

    vector< pair<string, size_t> > listControls();

    template <class T>
    void detect(T &img) {
        detect(ofxCv::toCv(img));
    }
    void detect(cv::Mat img);

    void processInteraction(const ofPoint &point);

    OscSender& getSender();

    static const ofColor accent1;
    static const ofColor accent2;

private:
    void assignControls();

    bool isButton(size_t i);
    bool isSlider(size_t i);
    bool isSwitch(size_t i);

    OscSender sender;

    ofxCv::ContourFinder finder;

    cv::Mat grayImg;
    cv::Mat edgesInput;
    cv::Mat edges;

    // Store the actual controls
    vector<Button*> buttons;
    vector<Slider*> sliders;
    vector<Switch*>  switches;

    // Store pointers for batch operations
    vector<Control*> controls;

    ofPoint lastInputPoint;

    vector<ofColor> colors;

    static const int BORDER = 8;
    static const float ALPHA = 0.7;
};
