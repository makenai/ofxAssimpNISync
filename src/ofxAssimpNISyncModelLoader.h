//
//  ofxAssimpModelLoader.cpp
//
//  Created by Ali Nakipoglu on 12/21/11.
//

#pragma once

#include "ofMain.h"

#include "ofxAssimpModelLoader.h"

typedef map<const aiBone*, aiMatrix4x4> BoneMatrices;

class ofxAssimpNISyncModelLoader : public ofxAssimpModelLoader 
{
    
public:
    
    ofxAssimpNISyncModelLoader();
    ~ofxAssimpNISyncModelLoader();
    
public:
    
    void updateSync( BoneMatrices& niBoneMatrices );
    
protected:
    
    BoneMatrices       mLatestBoneTransformations;
    
};
