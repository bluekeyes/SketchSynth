#include <math.h>

#include "Control.h"

//---------------------------------------------------------
RectControl::RectControl(const cv::RotatedRect &rotRect, OscSender &sender)
  : Control(sender) {
    const cv::Size &s = rotRect.size;
    bool switchHeight = s.width < s.height;

    if (switchHeight) {
        rect.setFromCenter(ofxCv::toOf(rotRect.center), s.height, s.width);
        angle = rotRect.angle + 90;
    } else {
        rect.setFromCenter(ofxCv::toOf(rotRect.center), s.width, s.height);
        angle = rotRect.angle;
    }
}

//---------------------------------------------------------
bool RectControl::contains(float x, float y) {
    ofVec2f v = alignPoint(x, y);
    return rect.inside(v.x, v.y);
}

//---------------------------------------------------------
bool RectControl::operator==(const RectControl &other) {
    // TODO Define constants for thresholds
    return (abs(other.angle - angle) < 10)
        && (abs(other.rect.x - rect.x) < 10)
        && (abs(other.rect.y - rect.y) < 10)
        && (abs(other.rect.width - rect.width) < 10)
        && (abs(other.rect.height - rect.height) < 10);
};

//---------------------------------------------------------
ofVec2f RectControl::alignPoint(float x, float y) {
    ofPoint center = rect.getCenter();
    ofVec2f v = ofVec2f(x - center.x, y - center.y);
    v.rotate(-angle);
    v += ofVec2f(rect.x + rect.width / 2, rect.y + rect.height / 2);
    return v;
}

//---------------------------------------------------------
Button::Button(float x, float y, float r, OscSender &sender)
  : Control(sender) {
    cx = x;
    cy = y;

    radius = r;
    active = false;
    entered = false;
}

//---------------------------------------------------------
void Button::draw() {
    ofSetColor(color);
    ofSetLineWidth(5);
    if (active) {
        ofFill();
    } else {
        ofNoFill();
    }
    ofCircle(cx, cy, radius);
}

//---------------------------------------------------------
bool Button::contains(float x, float y) {
    return ofDistSquared(cx, cy, x, y) < radius * radius;
}

//---------------------------------------------------------
bool Button::onInteraction(float x, float y) {
    if (contains(x, y)) {
        if (!entered) {
            entered = true;
            active = true;
            sender.sendMomentaryValue(id, active);
        }
        return true;
    } else if (entered) {
        active = false;
        entered = false;
        sender.sendMomentaryValue(id, active);
        return true;
    }

    return false;
}

//---------------------------------------------------------
bool Button::operator==(const Button &other) {
    // TODO Define constants for thresholds
    return (abs(other.cx - cx) < 10)
        && (abs(other.cy - cy) < 10)
        && (abs(other.radius - radius) < 10);
};


//---------------------------------------------------------
Slider::Slider(const cv::RotatedRect &rotRect, OscSender &sender)
  : RectControl(rotRect, sender) {
    value = 0;
}

//---------------------------------------------------------
void Slider::draw() {
    ofPushMatrix();
    ofTranslate(rect.getCenter());
    ofRotate(angle);

    ofNoFill();
    ofSetLineWidth(4);
    ofSetColor(color);

    // Draw the main shape
    ofLine(-rect.width / 2, 0, rect.width / 2, 0);
    ofLine(-rect.width / 2, -rect.height / 2, -rect.width / 2, rect.height / 2);
    ofLine(rect.width / 2, -rect.height / 2, rect.width / 2, rect.height / 2);
    
    // Draw the tick marks
    ofSetLineWidth(2);
    for (size_t i = 1; i < 8; i++) {
        float dx = ((float) i / 8) * rect.width;
        ofLine(-rect.width / 2 + dx, -rect.height / 3, -rect.width / 2 + dx, rect.height / 3);
    }

    // Draw the value marker
    float xval = value * rect.width;
    float knobHeight = 5 * rect.height / 4;
    float knobWidth = rect.width / 8;
    // Fill the rectangle with black to erase the background
    ofPushStyle();
    ofSetColor(0);
    ofFill();
    ofRect(-rect.width / 2 + xval - knobWidth / 2, -knobHeight / 2, knobWidth, knobHeight);
    ofPopStyle();

    ofSetLineWidth(5);
    ofRect(-rect.width / 2 + xval - knobWidth / 2, -knobHeight / 2, knobWidth, knobHeight);
    // ofCircle(-rect.width / 2 + xval, 0, 3 * rect.height / 8);
    
    ofPopMatrix();
}


//---------------------------------------------------------
bool Slider::onInteraction(float x, float y) {
    if (contains(x, y)) {
        ofVec2f v = alignPoint(x, y);
        value = (float) (v.x - rect.x) / rect.width;
        sender.sendContinuousValue(id, value);
        return true;
    }
    return false;
}

//---------------------------------------------------------
Switch::Switch(const cv::RotatedRect &rotRect, OscSender &sender) 
  : RectControl(rotRect, sender) {
      active = false;
}

//---------------------------------------------------------
void Switch::draw() {
    ofPushMatrix();
    ofTranslate(rect.getCenter());
    ofRotate(angle);

    ofNoFill();
    ofSetLineWidth(3);
    ofSetColor(color);

    ofRect(-rect.width / 2, -rect.height / 2, rect.width, rect.height);
    ofLine(0, -rect.height / 2, 0, rect.height / 2);

    float x = (active ? 0 : -rect.width / 2);
    ofLine(x, -rect.height / 2, x + rect.width / 2, rect.height / 2);
    ofLine(x + rect.width / 2, -rect.height / 2, x, rect.height / 2);

    ofPopMatrix();
}


//---------------------------------------------------------
bool Switch::onInteraction(float x, float y) {
    if (contains(x, y)) {
        ofVec2f pt = alignPoint(x, y);
        bool right = pt.x > rect.getCenter().x;

        if (active != right) {
            active = right;
            sender.sendToggleValue(id, active);
        }
        return true;
    }

    return false;
}
