//
//  ofxAssimpOpenNISkeletonSync.cpp
//
//  Created by Ali Nakipoglu on 12/21/11.
//

#include <iostream>

#include "ofxAssimpOpenNISkeletonSync.h"

ofxAssimpOpenNISkeletonSync::ofxAssimpOpenNISkeletonSync()
{
    mUserGenerator              = NULL;
    mModelLoader                = NULL;
    
    mSetup                      = false;
    mBoneListCreated            = false;
    
    mRequiredJointConfidence    = DEFAULT_REQUIRED_JOINT_CONFIDENCE;
};

ofxAssimpOpenNISkeletonSync::~ofxAssimpOpenNISkeletonSync()
{};

void ofxAssimpOpenNISkeletonSync::setup(ofxAssimpModelLoader *model, ofxUserGenerator *userGenerator)
{
    mModelLoader        = model;
    mUserGenerator      = userGenerator;
    
    mSetup              = true;
}

ofxUserGenerator* ofxAssimpOpenNISkeletonSync::getUserGenerator()
{
    return mUserGenerator;
}

ofxAssimpModelLoader* ofxAssimpOpenNISkeletonSync::getModel()
{
    return mModelLoader;
}

void ofxAssimpOpenNISkeletonSync::update()
{
    if ( !mSetup )
    {
        ofLog( OF_LOG_ERROR, "ofxAssimpOpenNISkeletonSync Cannot update. Setup must be called first!" );
        
        return;
    }
    
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
                        XnSkeletonJointPosition jointPosition;
                        
                        mUserGenerator->getXnUserGenerator().GetSkeletonCap().GetSkeletonJointOrientation( trackedUser->id, joint, jointOrientation );
                        mUserGenerator->getXnUserGenerator().GetSkeletonCap().GetSkeletonJointPosition( trackedUser->id, joint, jointPosition );
                        
                        if ( jointPosition.fConfidence > mRequiredJointConfidence )
                        {
                            float*  orientationData         = jointOrientation.orientation.elements;
                            
                            aiMatrix4x4 currentJointAIMat(orientationData[0], orientationData[1], orientationData[2], 0.0f,
                                                          orientationData[3], orientationData[4], orientationData[5], 0.0f,
                                                          orientationData[6], orientationData[7], orientationData[8], 0.0f,
                                                          0.0f, 0.0f, 0.0f, 1.0f);
                            
                            
                            aiQuaternion    currentBoneOri;
                            aiQuaternion    currentJointOri;
                            aiQuaternion    resultOri;
                            
                            aiVector3D      currentBonePos;
                            aiVector3D      currentJointPos;
                            
                            aiVector3D      currentBoneScale;
                            aiVector3D      currentJointScale;
                            
                            bone->mOffsetMatrix.Decompose(currentBoneScale, currentBoneOri, currentBonePos);
                            currentJointAIMat.Decompose(currentJointScale, currentJointOri, currentJointPos);
                            
                            ofQuaternion    newBoneOri( currentBoneOri.x, currentBoneOri.y, currentBoneOri.z, currentBoneOri.w );
                            ofQuaternion    newJointOri( currentJointOri.x, currentJointOri.y, currentJointOri.z, currentJointOri.w );
                            
                            ofMatrix4x4     newMatrix;
                            
                            newMatrix.rotate(newBoneOri * newJointOri);
                            newMatrix.translate( currentBonePos.x, currentBonePos.y, currentBonePos.y);
                            newMatrix.scale(currentBoneScale.x, currentBoneScale.y, currentBoneScale.z);
                            
                            ofVec4f         *mat            = newMatrix._mat;
                            
                            aiMatrix4x4     resultMatrix(   mat[0][0], mat[0][1], mat[0][2], mat[0][3],
                                                            mat[1][0], mat[1][1], mat[1][2], mat[1][3],
                                                            mat[2][0], mat[2][1], mat[2][2], mat[2][3],
                                                            mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
                            
                            bone->mOffsetMatrix             = resultMatrix;
                        }
                    }
                }

            }
        }
    }
}

void ofxAssimpOpenNISkeletonSync::setRequiredJointConfidence(float confidence)
{
    mRequiredJointConfidence    = confidence;
}

void ofxAssimpOpenNISkeletonSync::syncBoneAndOpenNIJoint(unsigned int meshIndex, string boneName, unsigned int userID, XnSkeletonJoint joint)
{
    if ( !mSetup )
    {
        ofLog( OF_LOG_ERROR, "ofxAssimpOpenNISkeletonSync Cannot sync bone. Setup must be called first!" );
        
        return;
    }
    
    if ( meshIndexIsValid( meshIndex ) )
    {
        if ( meshIsValid( meshIndex ) )
        {
            if ( !boneNameIsValid( meshIndex, boneName ) )
            {
                ofLog( OF_LOG_ERROR, "ofxAssimpOpenNISkeletonSync given bone name :" + boneName + " not found in meshIndex : " + ofToString( meshIndex) );
                
                return;
            }
        } else {
            ofLog( OF_LOG_ERROR, "ofxAssimpOpenNISkeletonSync given meshIndex :" + ofToString(meshIndex) + " has no bones" );
            
            return;
        }
    } else {
        ofLog( OF_LOG_ERROR, "ofxAssimpOpenNISkeletonSync given meshIndex :" + ofToString(meshIndex) + " not found" );
        
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

void ofxAssimpOpenNISkeletonSync::removeUserSync(int userID)
{
    if ( mSyncData.count( userID ) != 0 )
    {
        mSyncData.erase( userID );
    }
}

void ofxAssimpOpenNISkeletonSync::removeBoneSync(string boneName)
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

void ofxAssimpOpenNISkeletonSync::removeJointSync(XnSkeletonJoint joint)
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

void ofxAssimpOpenNISkeletonSync::removeMeshSync(int meshIndex)
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

void ofxAssimpOpenNISkeletonSync::removeAllSyncs()
{
    mSyncData.clear();
}

void ofxAssimpOpenNISkeletonSync::listBoneNames()
{
    const vector<string> boneNames      = getBoneNames();
    
    vector<string>::const_iterator it   = boneNames.begin();
    
    for ( ; it != boneNames.end(); ++it )
    {
        cout << "ofxAssimpOpenNISkeletonSync Listing Bone Name: " << *it << endl;
    }
}

const vector<string>& ofxAssimpOpenNISkeletonSync::getBoneNames()
{
    if ( !mBoneListCreated )
    {
        buildBoneNameList();
    }
    
    return mBoneNames;
}

const aiScene* ofxAssimpOpenNISkeletonSync::getAiScene(ofxAssimpModelLoader *model)
{
    return mModelLoader->getAssimpScene();
}

aiBone* ofxAssimpOpenNISkeletonSync::getBoneNamed(aiMesh *mesh, string boneName)
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

void ofxAssimpOpenNISkeletonSync::buildBoneNameList()
{
    if ( !mSetup )
    {
        ofLog( OF_LOG_ERROR, "ofxAssimpOpenNISkeletonSync Cannot build bone name list. Setup must be called first!" );
        
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

bool ofxAssimpOpenNISkeletonSync::meshIndexIsValid(unsigned int meshIndex)
{
    return meshIndex < getAiScene( mModelLoader )->mNumMeshes;
}

bool ofxAssimpOpenNISkeletonSync::meshIsValid(unsigned int meshIndex)
{
    return getAiScene( mModelLoader )->mMeshes[ meshIndex ]->HasBones();
}

bool ofxAssimpOpenNISkeletonSync::boneNameIsValid( unsigned int meshIndex, string boneName )
{    
    return getBoneNamed( getAiScene( mModelLoader )->mMeshes[ meshIndex ], boneName ) != NULL;
}