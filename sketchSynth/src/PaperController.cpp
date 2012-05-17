#include "ofxXmlSettings.h"

#include "PaperController.h"

#include "ShapeUtils.h"

using namespace ofxCv;
using namespace cv;

//---------------------------------------------------------
void PaperController::setup() {
    ofEnableSmoothing();
    ofBackground(0);

	paperCam.initGrabber(640, 480);
    paperCam.listDevices();

    foundPaper = false;
    paperDetector.setup();
	unwarped.allocate(518, 400, OF_IMAGE_COLOR);

    controlManager.setup();

    topBackground.setLearningTime(1800);
    topBackground.setThresholdValue(40);
    topBackground.setDifferenceMode(RunningBackground::ABSDIFF);

    doControlDetection = 0;

    debugDraw = false;

    // Start the app in setup mode
    if (!loadProjectorAlignment()) {
        ofLog(OF_LOG_NOTICE, "No saved alignment found, starting from scratch.");
        resetProjectorAlignment();
    }
    setupMode();
}

//---------------------------------------------------------
void PaperController::exit() {
    controlManager.getSender().sendStopAll();
}

//---------------------------------------------------------
void PaperController::update() {
    switch (state) {
        case EDIT:
            editUpdate();
            break;
        case SETUP:
            setupUpdate();
            break;
        case PLAY:
            playUpdate();
            break;
        default:
            break;
    }
}


//---------------------------------------------------------
void PaperController::setupUpdate() {
    paperCam.update();

    if (paperCam.isFrameNew()) {
	    foundPaper = paperDetector.detect(paperCam);
    }

    if (!alignmentComplete && projectorPoints.size() == 4) {
        computeProjectorAlignment();
        if (!saveProjectorAlignment()) {
            ofLog(OF_LOG_WARNING, "Could not save alignment.xml, projector alignment will be lost on exit.");
        }
    }
}

//---------------------------------------------------------
void PaperController::editUpdate() {
    paperCam.update();
}


//---------------------------------------------------------
void PaperController::playUpdate() {
    // Update the paper monitoring camera
    paperCam.update();

    // If we have a new frame and enough time as passed
    int time = ofGetElapsedTimeMillis();
	if (paperCam.isFrameNew() && time - playStartTime > toPlayDelay) {
        /*
         * TODO When should we do this? We need unwarped images for accurate
         * hand coordinates, but hands can distort the bounding rectangle. Can
         * we assume that the paper doesn't move in play mode?
         */
        // Look for paper and if we have it, unwarp the image
	    bool paper = paperDetector.detect(paperCam);
        foundPaper = foundPaper || paper;
        if (foundPaper) {
            paperDetector.unwarp(unwarped);
            unwarped.update();
        }

        if (doControlDetection) {
            controlManager.detect(unwarped);
            doControlDetection = false;
        }

        Mat paperCamMat = toCv(paperCam);
        Mat paperCamChan = Mat::zeros(paperCam.height, paperCam.width, CV_8UC3);
        int fromTo[] = { 1,0 , 1,1 , 1,2 };
        mixChannels(&paperCamMat, 1, &paperCamChan, 1, fromTo, 3);

        topBackground.update(paperCamChan, foreground);
        if (handDetector.detect(foreground, paperDetector.getPaper())) {
            ofPoint rawPoint = handDetector.getFingerPoint();
            controlManager.processInteraction(paperDetector.unwarpPoint(rawPoint, unwarped.width, unwarped.height));
        }
    }
}


//---------------------------------------------------------
void PaperController::playMode() {
    if (state == EDIT || state == SETUP) {
        // Reset and redetect controls
        controlManager.reset();
        doControlDetection = true;

        // Reset the background to the current image
        topBackground.reset();

        // Look for new paper, or paper in a new position
        foundPaper = false;

        playStartTime = ofGetElapsedTimeMillis();

        // TODO Reset and restart audio
    }
    state = PLAY;
}


//---------------------------------------------------------
void PaperController::editMode() {
    if (state == PLAY) {
        controlManager.getSender().sendStopAll();
    }
    state = EDIT;
}


//---------------------------------------------------------
void PaperController::setupMode() {
    if (state == PLAY) {
        controlManager.getSender().sendStopAll();
    }

    state = SETUP;
}


//---------------------------------------------------------
void PaperController::draw() {
    switch (state) {
        case EDIT:
            editDraw();
            break;
        case SETUP:
            setupDraw();
            break;
        case PLAY:
            playDraw();
            break;
        default:
            break;
    }
}

//---------------------------------------------------------
void PaperController::infoDraw() {
    int xp = padding / 2;
    int yp = padding;

    ofSetColor(20);
    ofFill();
    ofRect(0, 0, screenSeparation, ofGetHeight());

    // Draw the live camera feed
    ofSetLineWidth(1);
    ofSetColor(255);
    ofDrawBitmapString("Camera", xp, yp - 5);
    if (!debugDraw) {
        paperCam.draw(xp, yp, 320, 240);
    } else {
        Mat paperCamMat = toCv(paperCam);
        Mat paperCamChannel = Mat::zeros(paperCam.height, paperCam.width, CV_8UC3);
        int fromTo[] = { 1,0 , 1,1 , 1,2 };
        mixChannels(&paperCamMat, 1, &paperCamChannel, 1, fromTo, 3);
        drawMat(paperCamChannel, xp, yp, 320, 240);
    }

    // Draw paper and hand overlays on camera
    ofPushMatrix();
    ofTranslate(xp, yp);
    ofScale(0.5, 0.5);
    paperDetector.draw();
    ofSetColor(0, 255, 0);
    handDetector.draw();
    ofPopMatrix();

    yp += (240 + padding);

    // Draw control detector input
    ofSetColor(255);
    ofDrawBitmapString("Control Detector", xp, yp - 5);
    controlManager.drawDetectorInput(xp, yp, 320, 240);

    yp += (240 + padding);

    const vector< pair<string, size_t> > &controls = controlManager.listControls();
    stringstream controlStream;
    for (size_t i = 0; i < controls.size(); i++) {
        controlStream << controls[i].second << " " << controls[i].first << endl;
    }
    ofDrawBitmapString(controlStream.str(), xp, yp - 5);

    // Draw performance statistics
    ofDrawBitmapString(ofToString((int) ofGetFrameRate()) + " fps", xp, ofGetHeight() - 10);
    if (foundPaper) {
        ofDrawBitmapString("Paper detected", xp, ofGetHeight() - padding - 10);
    } else {
        ofDrawBitmapString("No paper", xp, ofGetHeight() - padding - 10);
    }

    yp = padding;
    xp += (320 + padding);

    // Draw processed background subtraction
    ofDrawBitmapString("BackSub Raw", xp, yp - 5);
    drawMat(foreground, xp, yp, 320, 240);
    yp += (240 + padding);

    // Draw processed background subtraction
    ofDrawBitmapString("BackSub Processed", xp, yp - 5);
    handDetector.drawDetectorInput(xp, yp, 320, 240);
}

//---------------------------------------------------------
void PaperController::playDraw() {
    infoDraw();

    //----------------------------//
    //   Draw on the projector    //
    //----------------------------//

    ofTranslate(screenSeparation, 0);
    ShapeUtils::applyTransform(toProjectorMatrix);

    // This push and pop is only needed because we're (possibly) drawing the paper outline
    ofPushMatrix();
    if (foundPaper) {
        ShapeUtils::applyTransform(paperDetector.getTransformation(unwarped.width, unwarped.height));
    }
    controlManager.drawControls();
    ofPopMatrix();

    // Outline the paper for debugging
    if (debugDraw) {
        ofNoFill();
        ofSetColor(255, 0, 0);
        ofSetLineWidth(2);
        paperDetector.getPaper().draw();
    }
}

//---------------------------------------------------------
void PaperController::editDraw() {
    infoDraw();
}

//---------------------------------------------------------
void PaperController::setupDraw() {
    ofSetLineWidth(1);
    ofSetColor(255);
    paperCam.draw(0, 0);
    paperDetector.draw();

    ofSetColor(0, 255, 0);
    ofFill();
    for (size_t i = 0; i < projectorPoints.size(); i++) {
        Point2f &pt = projectorPoints[i];
        ofCircle(pt.x, pt.y, 5);
    }

    // Draw performance statistics
    ofSetColor(255);
    ofDrawBitmapString(ofToString((int) ofGetFrameRate()) + " fps", 10, ofGetHeight() - 10);
    if (foundPaper) {
        ofDrawBitmapString("Paper detected", 10, ofGetHeight() - padding - 10);
    } else {
        ofDrawBitmapString("No paper", 10, ofGetHeight() - padding - 10);
    }
    ofDrawBitmapString("Press 'r' to reset alignment", 10, ofGetHeight() - 2 * padding - 10);

    // Draw the projection rectangle
    ofTranslate(screenSeparation, 0);
    ofNoFill();
    ofSetLineWidth(6);
    ofRect(0, 0, projWidth, projHeight);

    if (projectorPoints.size() == 4) {
        ofSetColor(34, 183, 220);
        ShapeUtils::applyTransform(toProjectorMatrix);
        ofPolyline quad = ofxCv::toOf(projectorPoints);
        ofPoint c = ShapeUtils::getCentroid2D(quad);
        ofFill();
        ofCircle(c.x, c.y, 20);
        ofCircle(c.x + 50, c.y, 20);
        ofCircle(c.x, c.y + 50, 20);
        ofCircle(c.x - 50, c.y, 20);
        ofCircle(c.x, c.y - 50, 20);
    }
}


//---------------------------------------------------------
void PaperController::resetProjectorAlignment() {
    projectorPoints.clear();
    toProjectorMatrix = Mat::eye(3, 3, CV_32F);
    alignmentComplete = false;
}

//---------------------------------------------------------
bool PaperController::saveProjectorAlignment() {
    if (projectorPoints.size() != 4) {
        return false;
    }

    ofxXmlSettings alignment;
    alignment.addTag("alignment");
    alignment.pushTag("alignment");
    for (size_t i = 0; i < projectorPoints.size(); i++) {
        alignment.addTag("position");
        alignment.pushTag("position", i);
        alignment.addValue("x", projectorPoints[i].x);
        alignment.addValue("y", projectorPoints[i].y);
        alignment.popTag();
    }
    alignment.popTag();

    // TODO If ever upgraded beyond oF 0.700, this returns a boolean
    alignment.saveFile("alignment.xml");
    return true;
}

//---------------------------------------------------------
bool PaperController::loadProjectorAlignment() {
    ofxXmlSettings alignment;
    if (!alignment.loadFile("alignment.xml")) {
        return false;
    }

    alignment.pushTag("alignment");
    if (alignment.getNumTags("position") != 4) {
        return false;
    }

    projectorPoints.clear();
    for (size_t i = 0; i < 4; i++) {
        alignment.pushTag("position", i);

        Point2f p;
        p.x = alignment.getValue("x", 0);
        p.y = alignment.getValue("y", 0);
        projectorPoints.push_back(p);

        alignment.popTag();
    }
    alignment.popTag();

    computeProjectorAlignment();
    return true;
}

//---------------------------------------------------------
void PaperController::computeProjectorAlignment() {
    vector<Point2f> dstPoints(4);
    dstPoints[0] = Point2f(0, 0);
    dstPoints[1] = Point2f(projWidth, 0);
    dstPoints[2] = Point2f(projWidth, projHeight);
    dstPoints[3] = Point2f(0, projHeight);

    // This matrix transforms from point in camera space to points in
    // projector space, i.e. if point a is at (x, y) as seen by the camera,
    // transforming point b at (2x, 2y) will make it appear correct when
    // projected back into the scene.
    toProjectorMatrix = getPerspectiveTransform(&projectorPoints[0], &dstPoints[0]);
    alignmentComplete = true;
}

//---------------------------------------------------------
void PaperController::keyPressed(int key) {
    switch (key) {
        case 'e':
            editMode();
            break;
        case 'p':
            playMode();
            break;
        case 's':
            setupMode();
            break;
        case 'r':
            if (state == SETUP) {
                resetProjectorAlignment();
            }
            break;
        case 'f':
            ofToggleFullscreen();
            ofSetWindowShape(2128, 800);
            break;
        case 'd':
            debugDraw = !debugDraw;
            break;
        default:
            break;
    }
}

//---------------------------------------------------------
void PaperController::mousePressed(int x, int y, int button) {
    if (state == SETUP && projectorPoints.size() < 4) {
        projectorPoints.push_back(Point2f(x, y));
    }
}
