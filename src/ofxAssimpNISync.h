//
//  ofxAssimpNISync.h
//
//  Created by Ali Nakipoglu on 12/21/11.
//

#pragma once

#include "ofMain.h"

#include "ofxOpenNI.h"
#include "ofxAssimpModelLoader.h"
#include "ofxAssimpNISyncModelLoader.h"

/*
 *  Default value assigned to check OpenNI joints.
 */
#define DEFAULT_REQUIRED_JOINT_CONFIDENCE 0.3f


class ofxAssimpNISync
{
    
public:
    
    // Constructor & Deconstructor
    
    ofxAssimpNISync();
    ~ofxAssimpNISync();
    
public:
    
    /*
     *  Setups synchronization. 
     *  model and userGenerator pointers must be valid and this function call is required.
     */
    void                        setup( ofxAssimpNISyncModelLoader* model, ofxOpenNI* device );
    
    /*
     *  Returns ofxOpenNIUserGenerator
     */
    ofxOpenNI*           getDevice();
    
    /*
     *  Return AssimModelLoader
     */
    ofxAssimpNISyncModelLoader* getModel();
    
    /*
     *  Updates all synchronizations.
     *  For successful update, the processed synchronization must have a
     *  user skeleton tracked and available joint & corresponding mesh/bone pairs found.
     */
    void                        update();
    
    /*
     *  Sets default joint confidence to check OpenNI joints.
     */
    void                        setRequiredJointConfidence( float confidence );
    
    /*
     *  Adds a synchronization. 
     */
    void                        syncBoneAndOpenNIJoint( unsigned int meshIndex, string boneName, unsigned int userID, XnSkeletonJoint joint );
    
    //  Handy synchronization removal methods.
    
    void                        removeUserSync( int userID );
    void                        removeBoneSync( string boneName );
    void                        removeJointSync( XnSkeletonJoint joint );
    void                        removeMeshSync( int meshIndex );
    
    void                        removeAllSyncs();
    
    /*
     *  List available bone name in given aiScene.
     */
    void                        listBoneNames();
    
    /*
     *  Resurns list of available bone name in given aiScene.
     */
    const vector<string>&       getBoneNames();
    
protected:
    
    /*
     *  Return aiScene in current AssimpModelLoader 
     */
    inline const aiScene*       getAiScene( ofxAssimpModelLoader* model );
    
    /*
     *  Return aiBone pointer by given bone name and mesh pointer
     */
    
    inline aiBone*              getBoneNamed( aiMesh* mesh, string boneName );
    
    /*
     *  Builds bone names list.
     */
    void                        buildBoneNameList();
    
    // Utility functions to check given value validity
    
    inline bool                 meshIndexIsValid( unsigned int meshIndex );
    inline bool                 meshIsValid( unsigned int meshIndex );
    inline bool                 boneNameIsValid( unsigned int meshIndex, string boneName );
    
protected:
    
    // typedefs to hold synchronizations between bone, joint, mesh and user
    
    typedef map<string, XnSkeletonJoint>        PerBoneSyncData;
    typedef map<unsigned int, PerBoneSyncData>  PerMeshSyncData;
    typedef map<unsigned int, PerMeshSyncData>  PerUserSyncData;
    
    // Synchronization data
    
    PerUserSyncData                             mSyncData;
    
    // Pointer to required userGenerator and aiScene
    
    ofxOpenNI*                                  openNIDevice;
    ofxAssimpNISyncModelLoader*                 mModelLoader;
    
    /*
     *  Bool to hold setup method called or not.
     */    
    bool                                        mSetup;
    
    /*
     *  Bool to check if bone name lists are ready or not.
     */
    bool                                        mBoneListCreated;
    
    /*
     *  List of bone names  string( mBones[ j ]->mName.data )
     */
    vector<string>                              mBoneNames;
    
    /*
     *  Required confidence value to check OpenNI joints
     */
    float                                       mRequiredJointConfidence;
    
    /*
     *  Map to hold new per bone matrices
     */
    BoneMatrices                                mBoneMatrices;
};
