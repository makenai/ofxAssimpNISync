//
//  ofxAssimpNISyncLoader.h
//  InteractiveOutdoor
//
//  Created by Ali Nakipoglu on 12/23/11.
//  Copyright (c) 2011 Publicis Modem Turkiye. All rights reserved.
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
