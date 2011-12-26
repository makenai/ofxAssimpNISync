#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup()
{
    // Set default OF settings.
    
    ofSetVerticalSync( true );
    ofSetFrameRate( 60 );
    
    ofSetWindowShape( 1280, 720 );
    ofSetWindowTitle( "ofxAssimpNISyncExample" );
    
    ofBackground( 0, 0, 0 );
    
    ofSetLogLevel( OF_LOG_VERBOSE );
    
    // First setup hardware if we are on mac.
    
#if defined (TARGET_OSX)
    niHardwareDriver.setup();
#endif
    
    // Init OpenNI Objects.
    
    niContext.setup();
    
    niDepthGenerator.setup( &niContext );
    niImageGenerator.setup( &niContext );
    niUserGenerator.setup( &niContext );
    
    // Init ofxAssimpNISync.
    
    // Set required environment settings and load the model.

    
    ofDisableArbTex();
    ofEnableBlendMode( OF_BLENDMODE_ALPHA );
    
    modelLoader.loadModel( MODEL_FILE_PATH );
    
    light.enable();
    
    glEnable( GL_DEPTH_TEST );
    ofEnableSeparateSpecularLight();
    
    // Setup AssimpNISync.
    
    assimpNISync.setup( &modelLoader, &niUserGenerator);
    
    /*  List Bone Names.
        Use these names to apply syncs below.*/
        
    assimpNISync.listBoneNames();
    
    // Apply syncs.
    
    /*
     
     Tip : Available OpenNI Joints
     
     XN_SKEL_HEAD
     XN_SKEL_NECK
     XN_SKEL_TORSO
     XN_SKEL_WAIST
     
     XN_SKEL_LEFT_COLLAR
     XN_SKEL_LEFT_SHOULDER
     XN_SKEL_LEFT_ELBOW
     XN_SKEL_LEFT_WRIST
     XN_SKEL_LEFT_HAND
     XN_SKEL_LEFT_FINGERTIP
     
     XN_SKEL_RIGHT_COLLAR
     XN_SKEL_RIGHT_SHOULDER
     XN_SKEL_RIGHT_ELBOW
     XN_SKEL_RIGHT_WRIST
     XN_SKEL_RIGHT_HAND
     XN_SKEL_RIGHT_FINGERTIP
     
     XN_SKEL_LEFT_HIP
     XN_SKEL_LEFT_KNEE
     XN_SKEL_LEFT_ANKLE
     XN_SKEL_LEFT_FOOT
     
     XN_SKEL_RIGHT_HIP
     XN_SKEL_RIGHT_KNEE
     XN_SKEL_RIGHT_ANKLE
     XN_SKEL_RIGHT_FOOT
     
     */
    
    // All head bones
    
    assimpNISync.syncBoneAndOpenNIJoint( 0, "head", 1, XN_SKEL_HEAD );
    assimpNISync.syncBoneAndOpenNIJoint( 1, "head", 1, XN_SKEL_HEAD );
    assimpNISync.syncBoneAndOpenNIJoint( 2, "head", 1, XN_SKEL_HEAD );
    assimpNISync.syncBoneAndOpenNIJoint( 3, "head", 1, XN_SKEL_HEAD );
    
    // All neck bones
    
    assimpNISync.syncBoneAndOpenNIJoint( 0, "neck01", 1, XN_SKEL_NECK );
    assimpNISync.syncBoneAndOpenNIJoint( 2, "neck01", 1, XN_SKEL_NECK );
    assimpNISync.syncBoneAndOpenNIJoint( 3, "neck01", 1, XN_SKEL_NECK );
    
    // All Shoulder bones
    
    assimpNISync.syncBoneAndOpenNIJoint( 3, "L_shoulder", 1, XN_SKEL_LEFT_SHOULDER );
    assimpNISync.syncBoneAndOpenNIJoint( 3, "R_shoulder", 1, XN_SKEL_RIGHT_SHOULDER );
    
    // All Elbow bones
    
    assimpNISync.syncBoneAndOpenNIJoint( 3, "L_elbow", 1, XN_SKEL_LEFT_ELBOW );
    assimpNISync.syncBoneAndOpenNIJoint( 3, "R_elbow", 1, XN_SKEL_RIGHT_ELBOW );
    
    // All Wrist bones
    
    assimpNISync.syncBoneAndOpenNIJoint( 2, "L_wrist", 1, XN_SKEL_LEFT_WRIST );
    assimpNISync.syncBoneAndOpenNIJoint( 2, "R_wrist", 1, XN_SKEL_RIGHT_WRIST );
    
    // All Hip bones
    
    assimpNISync.syncBoneAndOpenNIJoint( 3, "L_hip", 1, XN_SKEL_LEFT_HIP );
    assimpNISync.syncBoneAndOpenNIJoint( 3, "R_hip", 1, XN_SKEL_RIGHT_HIP );
    
    // init Kinect tilt Angle

#if defined (TARGET_OSX)    
    tiltAngle       = niHardwareDriver.getTiltAngle();
#endif
}

//--------------------------------------------------------------
void testApp::update()
{
    // if we are on mac update OpenNI Hardware driver.
    
#if defined (TARGET_OSX)
    niHardwareDriver.update();
    
    niHardwareDriver.setTiltAngle( tiltAngle );
#endif
    
    // Update OpenNI.
    
    niContext.update();
    niDepthGenerator.update();
    niImageGenerator.update();
    niUserGenerator.update();
    
    // Update AssimpNISync
    assimpNISync.update();
}

//--------------------------------------------------------------
void testApp::draw()
{
    if ( niUserGenerator.getNumberOfTrackedUsers() > 0 )
    {
        if ( niUserGenerator.getTrackedUser( 1 )->skeletonTracking )
        {
            ofSetColor( 255, 255, 255 );
            ofDrawBitmapString( "Tracking User Skeleton", 10.0f, 20.0f );
        } else if ( niUserGenerator.getTrackedUser( 1 )->skeletonCalibrating )
        {
            ofSetColor( 0, 255, 255 );
            ofDrawBitmapString( "Calibrating User", 10.0f, 20.0f );
        }
        
    } else {
        ofSetColor( 255, 255, 255 );
        ofDrawBitmapString( "Calibration Pose Required" , 10.0f, 20.0f );        
        ofSetColor( 255, 0, 0 );
        ofDrawBitmapString( "No Users Detected", 10.0f, 35.0f );
    }
    
    ofSetColor( 255, 255, 255, 255 );
    
    ofDrawBitmapString( "Press Up/Down to tilt Kinect" , 10.0f, ofGetWindowHeight() - 20.0f );
    
    ofPushMatrix();
    
    ofTranslate( modelLoader.getPosition().x, modelLoader.getPosition().y );
    ofTranslate( ofGetWindowWidth() / 2, ofGetWindowHeight() / 2 );
    ofTranslate( 0, 200 );
    
    ofRotate( -mouseX, 0, 1, 0 );
    
    modelLoader.drawFaces();
    
    ofPopMatrix();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
#if defined (TARGET_OSX)
    if ( key == 357 )
    {
        tiltAngle++;
    }
    
    if ( key == 359 )
    {
        tiltAngle--;
    }
#endif
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}