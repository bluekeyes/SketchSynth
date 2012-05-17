#pragma once

#include "ofMain.h"
#include "ofxCv.h"

#include "OscSender.h"

//---------------------------------------------------------
class Control {
public:
    Control(OscSender &sender) : id(-1), sender(sender) {}
    virtual ~Control() {}

    void setId(int id) {
        this->id = id;
    }

    virtual void draw() = 0;
    virtual void setColor(const ofColor &newColor) {
        color = newColor;
    }

    virtual bool contains(float x, float y) = 0;
    virtual bool contains(const ofPoint &point) {
        return contains(point.x, point.y);
    }

    virtual bool onInteraction(float x, float y) = 0;
    virtual bool onInteraction(const ofPoint &point) {
        return onInteraction(point.x, point.y);
    }

protected:
    int id;
    OscSender &sender;
    ofColor color;
};

//---------------------------------------------------------
class RectControl : public Control {
public:
    RectControl(const cv::RotatedRect &rotRect, OscSender &sender);

    bool contains(float x, float y);
    virtual bool operator==(const RectControl &other);
    bool operator!=(const RectControl &other) {
        return !(*this == other);
    }

protected:
    ofVec2f alignPoint(float x, float y);

    float angle;
    ofRectangle rect;
};

//---------------------------------------------------------
class Button : public Control {
public:
    Button(float x, float y, float r, OscSender &sender);

    void draw();
    bool contains(float x, float y);
    bool onInteraction(float x, float y);
    bool operator==(const Button &other);
    bool operator!=(const Button &other) {
        return !(*this == other);
    }

private:
    float cx;
    float cy;
    float radius;
    bool active;
    bool entered;
};

//---------------------------------------------------------
class Slider : public RectControl {
public:
    Slider(const cv::RotatedRect &rotRect, OscSender &sender);

    void draw();
    bool onInteraction(float x, float y);

private:
    float value;
};

//---------------------------------------------------------
class Switch : public RectControl {
public:
    Switch(const cv::RotatedRect &rotRect, OscSender &sender);

    void draw();
    bool onInteraction(float x, float y);

private:
    bool active;
};
