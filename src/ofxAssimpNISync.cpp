//
//  ofxAssimpNISync.cpp
//
//  Created by Ali Nakipoglu on 12/21/11.
//

#include <iostream>

#include "ofxAssimpNISync.h"

ofxAssimpNISync::ofxAssimpNISync()
{
    mUserGenerator              = NULL;
    mModelLoader                = NULL;
    
    mSetup                      = false;
    mBoneListCreated            = false;
    
    mRequiredJointConfidence    = DEFAULT_REQUIRED_JOINT_CONFIDENCE;
};

ofxAssimpNISync::~ofxAssimpNISync()
{};

void ofxAssimpNISync::setup(ofxAssimpNISyncModelLoader *model, ofxUserGenerator *userGenerator)
{
    mModelLoader        = model;
    mUserGenerator      = userGenerator;
    
    mSetup              = true;
}

ofxUserGenerator* ofxAssimpNISync::getUserGenerator()
{
    return mUserGenerator;
}

ofxAssimpNISyncModelLoader* ofxAssimpNISync::getModel()
{
    return mModelLoader;
}

void ofxAssimpNISync::update()
{
    if ( !mSetup )
    {
        ofLog( OF_LOG_ERROR, "ofxAssimpNISync Cannot update. Setup must be called first!" );
        
        return;
    }
    
    mBoneMatrices.clear();
    
    const aiScene* scene    = getAiScene( mModelLoader );
    
    PerUserSyncData::iterator   userIt  = mSyncData.begin();
    PerMeshSyncData::iterator   meshIt;
    PerBoneSyncData::iterator   boneIt;
    
    ofxTrackedUser* trackedUser;
    
    for ( ; userIt != mSyncData.end(); ++userIt )
    {
        trackedUser     = mUserGenerator->getTrackedUser( userIt->first );
        
        if ( trackedUser != NULL && trackedUser->skeletonTracking )
        {
            meshIt      = userIt->second.begin();
            
            for ( ; meshIt != userIt->second.end(); ++meshIt )
            {
                aiMesh* mesh    = scene->mMeshes[ meshIt->first ];
                
                boneIt          = meshIt->second.begin();
                
                for ( ; boneIt != meshIt->second.end(); ++boneIt )
                {
                    aiBone* bone    = getBoneNamed( mesh, boneIt->first );
                    
                    if ( bone )
                    {
                        XnSkeletonJoint joint   = boneIt->second;
                        
                        XnSkeletonJointOrientation  jointOrientation;
                        XnSkeletonJointPosition     jointPosition;
                        
                        mUserGenerator->getXnUserGenerator().GetSkeletonCap().GetSkeletonJointOrientation( trackedUser->id, joint, jointOrientation );
                        mUserGenerator->getXnUserGenerator().GetSkeletonCap().GetSkeletonJointPosition( trackedUser->id, joint, jointPosition );
                        
                        if ( jointPosition.fConfidence > mRequiredJointConfidence )
                        {
                            float*  orientationData         = jointOrientation.orientation.elements;
                            
                            mBoneMatrices[bone]             = aiMatrix4x4(  orientationData[0], orientationData[1], orientationData[2], 0.0f,
                                                                            orientationData[3], orientationData[4], orientationData[5], 0.0f,
                                                                            orientationData[6], orientationData[7], orientationData[8], 0.0f,
                                                                            0.0f, 0.0f, 0.0f, 1.0f );
                        }
                    }
                }

            }
        }
    }
    
    mModelLoader->updateSync( mBoneMatrices );
}

void ofxAssimpNISync::setRequiredJointConfidence(float confidence)
{
    mRequiredJointConfidence    = confidence;
}

void ofxAssimpNISync::syncBoneAndOpenNIJoint(unsigned int meshIndex, string boneName, unsigned int userID, XnSkeletonJoint joint)
{
    if ( !mSetup )
    {
        ofLog( OF_LOG_ERROR, "ofxAssimpNISync Cannot sync bone. Setup must be called first!" );
        
        return;
    }
    
    if ( meshIndexIsValid( meshIndex ) )
    {
        if ( meshIsValid( meshIndex ) )
        {
            if ( !boneNameIsValid( meshIndex, boneName ) )
            {
                ofLog( OF_LOG_ERROR, "ofxAssimpNISync given bone name :" + boneName + " not found in meshIndex : " + ofToString( meshIndex) );
                
                return;
            }
        } else {
            ofLog( OF_LOG_ERROR, "ofxAssimpNISync given meshIndex :" + ofToString(meshIndex) + " has no bones" );
            
            return;
        }
    } else {
        ofLog( OF_LOG_ERROR, "ofxAssimpNISync given meshIndex :" + ofToString(meshIndex) + " not found" );
        
        return;
    }
    
    if ( mSyncData.count( userID) == 0 )
    {
        mSyncData[ userID ]                         = PerMeshSyncData();
    }
    
    if ( mSyncData[ userID ].count( meshIndex ) == 0 )
    {
        mSyncData[ userID ][ meshIndex ]            = PerBoneSyncData();
    }
    
    mSyncData[ userID ][ meshIndex ][ boneName ]    = joint;
}

void ofxAssimpNISync::removeUserSync(int userID)
{
    if ( mSyncData.count( userID ) != 0 )
    {
        mSyncData.erase( userID );
    }
}

void ofxAssimpNISync::removeBoneSync(string boneName)
{
    PerUserSyncData::iterator   userIt  = mSyncData.begin();
    PerMeshSyncData::iterator   meshIt;
    
    for ( ; userIt != mSyncData.end(); ++userIt )
    {
        meshIt                          = userIt->second.begin();
        
        for ( ; meshIt != userIt->second.end(); ++meshIt )
        {
            if ( meshIt->second.count( boneName) != 0 )
            {
                meshIt->second.erase( boneName );
            }
        }
    }
}

void ofxAssimpNISync::removeJointSync(XnSkeletonJoint joint)
{
    PerUserSyncData::iterator   userIt  = mSyncData.begin();
    PerMeshSyncData::iterator   meshIt;
    PerBoneSyncData::iterator   boneIt;
    
    for ( ; userIt != mSyncData.end(); ++userIt )
    {
        meshIt                          = userIt->second.begin();
        
        for ( ; meshIt != userIt->second.end(); ++meshIt )
        {
            boneIt                      = meshIt->second.begin();
            
            for ( ; boneIt != meshIt->second.end(); ++boneIt )
            {
                if ( boneIt->second == joint )
                {
                    meshIt->second.erase( boneIt );
                }
            }
        }
    }
}

void ofxAssimpNISync::removeMeshSync(int meshIndex)
{
    PerUserSyncData::iterator   userIt  = mSyncData.begin();
    PerMeshSyncData::iterator   meshIt;
    
    for ( ; userIt != mSyncData.end(); ++userIt )
    {
        if ( userIt->second.count( meshIndex ) )
        {
            userIt->second.erase( meshIndex );
        }
    }
}

void ofxAssimpNISync::removeAllSyncs()
{
    mSyncData.clear();
}

void ofxAssimpNISync::listBoneNames()
{
    const vector<string> boneNames      = getBoneNames();
    
    vector<string>::const_iterator it   = boneNames.begin();
    
    for ( ; it != boneNames.end(); ++it )
    {
        cout << "ofxAssimpNISync Listing Bone Name: " << *it << endl;
    }
}

const vector<string>& ofxAssimpNISync::getBoneNames()
{
    if ( !mBoneListCreated )
    {
        buildBoneNameList();
    }
    
    return mBoneNames;
}

const aiScene* ofxAssimpNISync::getAiScene(ofxAssimpModelLoader *model)
{
    return mModelLoader->getAssimpScene();
}

aiBone* ofxAssimpNISync::getBoneNamed(aiMesh *mesh, string boneName)
{    
    for ( unsigned int i = 0; i < mesh->mNumBones; i++ )
    {
        if ( string( mesh->mBones[ i ]->mName.data ) == boneName )
        {
            return mesh->mBones[ i ];
        }
    }
    
    return NULL;
}

void ofxAssimpNISync::buildBoneNameList()
{
    if ( !mSetup )
    {
        ofLog( OF_LOG_ERROR, "ofxAssimpNISync Cannot build bone name list. Setup must be called first!" );
        
        return;
    }
    
    mBoneListCreated    = true;
    
    const aiScene*  aiScene = getAiScene( mModelLoader );

    for ( unsigned int i = 0; i < aiScene->mNumMeshes; i++ )
    {
        for ( unsigned int j = 0; j < aiScene->mMeshes[ i ]->mNumBones; j++ )
        {
            mBoneNames.push_back( string( aiScene->mMeshes[ i ]->mBones[ j ]->mName.data ) );
        }
    }
}

bool ofxAssimpNISync::meshIndexIsValid(unsigned int meshIndex)
{
    return meshIndex < getAiScene( mModelLoader )->mNumMeshes;
}

bool ofxAssimpNISync::meshIsValid(unsigned int meshIndex)
{
    return getAiScene( mModelLoader )->mMeshes[ meshIndex ]->HasBones();
}

bool ofxAssimpNISync::boneNameIsValid( unsigned int meshIndex, string boneName )
{    
    return getBoneNamed( getAiScene( mModelLoader )->mMeshes[ meshIndex ], boneName ) != NULL;
}