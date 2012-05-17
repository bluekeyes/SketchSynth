#include "ofMain.h"

#include "OscSender.h"

//---------------------------------------------------------
void OscSender::setup(string host, int port) {
    sender.setup(host, port);
}

//---------------------------------------------------------
void OscSender::sendStopAll() {
    ofxOscMessage m;
    m.setAddress("/paper/stop");
    sender.sendMessage(m);
}

//---------------------------------------------------------
void OscSender::sendStartAll() {
    ofxOscMessage m;
    m.setAddress("/paper/start");
    sender.sendMessage(m);
}

//---------------------------------------------------------
void OscSender::sendControlCount(ControlType type, int count) {
    ofxOscMessage m;
    m.setAddress("/paper/count");
    switch (type) {
        case CONTINUOUS:
            m.addStringArg("continuous");
            break;
        case TOGGLE:
            m.addStringArg("toggle");
            break;
        case MOMENTARY:
            m.addStringArg("momentary");
            break;
    }
    m.addIntArg(count);
    sender.sendMessage(m);
}

//---------------------------------------------------------
void OscSender::sendContinuousValue(int id, float value) {
    ofxOscMessage m;
    m.setAddress("/paper/continuous");
    m.addIntArg(id);
    m.addFloatArg(value);
    sender.sendMessage(m);
}

//---------------------------------------------------------
void OscSender::sendToggleValue(int id, bool state) {
    ofxOscMessage m;
    m.setAddress("/paper/toggle");
    m.addIntArg(id);
    m.addStringArg(state ? "on" : "off");
    sender.sendMessage(m);
}

//---------------------------------------------------------
void OscSender::sendMomentaryValue(int id, bool on) {
    ofxOscMessage m;
    m.setAddress("/paper/momentary");
    m.addIntArg(id);
    m.addStringArg(on ? "on" : "off");
    sender.sendMessage(m);
}
