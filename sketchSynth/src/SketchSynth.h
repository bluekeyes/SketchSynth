#pragma once

#include "ofMain.h"
#include "ofxCv.h"

#include "PaperDetector.h"
#include "ControlManager.h"
#include "HandDetector.h"

enum AppState { PLAY, EDIT, SETUP };

class SketchSynth : public ofBaseApp {
    public:
        void setup();
        void update();
        void draw();
        void exit();
        void keyPressed(int key);
        void mousePressed(int x, int y, int button);

    private:
        void setupUpdate();
        void setupDraw();

        void infoDraw();

        void editUpdate();
        void editDraw();

        void playUpdate();
        void playDraw();

        void playMode();
        void editMode();
        void setupMode();

        void resetProjectorAlignment();
        bool saveProjectorAlignment();
        bool loadProjectorAlignment();
        void computeProjectorAlignment();

        ofVideoGrabber paperCam;

        PaperDetector paperDetector;
        ofImage unwarped;

        HandDetector handDetector;

        bool foundPaper;
        AppState state;

        ofxCv::RunningBackground topBackground;
        cv::Mat foreground;

        int playStartTime;
        static const int toPlayDelay = 250;

        //--- CONTROL VARIABLE ---//
        ControlManager controlManager;
        bool doControlDetection;

        //--- SETUP VARIABLES ---//
        vector<cv::Point2f> projectorPoints;
        cv::Mat toProjectorMatrix;
        bool alignmentComplete;

        static const int projWidth = 848;
        static const int projHeight = 480;

        //--- DRAWING VARIABLES ---//
        static const int padding = 25;
        static const int screenSeparation = 1280;
        bool debugDraw;
};
