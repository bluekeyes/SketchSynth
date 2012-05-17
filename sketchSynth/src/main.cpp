#include "SketchSynth.h"
#include "ofAppGlutWindow.h"

int main() {
	ofAppGlutWindow window;
	ofSetupOpenGL(&window, 2128, 800, OF_FULLSCREEN);
	ofRunApp(new SketchSynth());
}
