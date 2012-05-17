#pragma once

#include "ofxOsc.h"

#define DEFAULT_HOST "localhost"
#define DEFAULT_PORT 12345

enum ControlType { CONTINUOUS, TOGGLE, MOMENTARY };

class OscSender {
public:
    void setup(string host = DEFAULT_HOST, int port = DEFAULT_PORT);

    void sendStopAll();
    void sendStartAll();
    void sendControlCount(ControlType type, int count);

    void sendContinuousValue(int id, float value);
    void sendToggleValue(int id, bool state);
    void sendMomentaryValue(int id, bool on);

private:
    ofxOscSender sender;
};
