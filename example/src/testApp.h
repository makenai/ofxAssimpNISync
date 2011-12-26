#pragma once

#include "ofMain.h"

#include "ofxOpenNI.h"
#include "ofxAssimpNISync.h"

#define MODEL_FILE_PATH "assimp_model/astroBoy_walk.dae"

class testApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
    
    // Default OpenNI Objects.
    
#if defined (TARGET_OSX)
    ofxHardwareDriver               niHardwareDriver;
#endif
    
    ofxOpenNIContext                niContext;
    ofxDepthGenerator               niDepthGenerator;
    ofxImageGenerator               niImageGenerator;
    ofxUserGenerator                niUserGenerator;
    
    // ofxAssimpNISync Objects.
    
    ofxAssimpNISyncModelLoader      modelLoader;
    ofxAssimpNISync                 assimpNISync;
    
    // Model's render related objects
    
    ofLight                         light;
    
    // Kinect Tilt Angle
#if defined (TARGET_OSX)    
    int                             tiltAngle;
#endif
};
