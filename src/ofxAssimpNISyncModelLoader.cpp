//
//  ofxAssimpModelLoader.cpp
//
//  Created by Ali Nakipoglu on 12/21/11.
//

#include <iostream>

#include "ofxAssimpNISyncModelLoader.h"

ofxAssimpNISyncModelLoader::ofxAssimpNISyncModelLoader()
{};

ofxAssimpNISyncModelLoader::~ofxAssimpNISyncModelLoader()
{};

void ofxAssimpNISyncModelLoader::updateSync( BoneMatrices &niBoneMatrices)
{
    // update mesh position for the animation
	for (unsigned int i = 0; i < modelMeshes.size(); ++i){
        
		// current mesh we are introspecting
		const aiMesh* mesh = modelMeshes[i].mesh;
        
		// calculate bone matrices
		std::vector<aiMatrix4x4> boneMatrices( mesh->mNumBones);
                
		for( size_t a = 0; a < mesh->mNumBones; ++a)
		{
			const aiBone* bone = mesh->mBones[a];
            
			// find the corresponding node by again looking recursively through the node hierarchy for the same name
			aiNode* node = scene->mRootNode->FindNode(bone->mName);
            
			// start with the mesh-to-bone matrix
			boneMatrices[a] = bone->mOffsetMatrix;
            
            aiMatrix4x4 newOrientation;
            bool newOrientationFound                = false;
            
            // Check a new mat assigned to this bone 
            if ( niBoneMatrices.count( bone ) != 0 )
            {
                newOrientation                      = niBoneMatrices[ bone ];
                mLatestBoneTransformations[ bone ]  = niBoneMatrices[ bone ];
                
                newOrientationFound                 = true;
                
            } else if ( mLatestBoneTransformations.count( bone ) != 0 )
            {
                // If not found use latest one if exist
                newOrientation                      = mLatestBoneTransformations[ bone ];
                
                newOrientationFound                 = true;
            }
             
			// and now append all node transformations down the parent chain until we're back at mesh coordinates again
			const aiNode* tempNode = node;
            
			while( tempNode)
			{
                if ( newOrientationFound )
                {
                        boneMatrices[a] = (tempNode->mTransformation * newOrientation) * boneMatrices[a];
                } else {
                        boneMatrices[a] = tempNode->mTransformation * boneMatrices[a];
                }

				tempNode = tempNode->mParent;
			}
			modelMeshes[i].hasChanged = true;
			modelMeshes[i].validCache = false;
		}
        
		modelMeshes[i].animatedPos.assign(modelMeshes[i].animatedPos.size(),0);
		if(mesh->HasNormals()){
			modelMeshes[i].animatedNorm.assign(modelMeshes[i].animatedNorm.size(),0);
		}
		// loop through all vertex weights of all bones
		for( size_t a = 0; a < mesh->mNumBones; ++a)
		{
			const aiBone* bone = mesh->mBones[a];
			const aiMatrix4x4& posTrafo = boneMatrices[a];
            
            
			for( size_t b = 0; b < bone->mNumWeights; ++b)
			{
				const aiVertexWeight& weight = bone->mWeights[b];
                
				size_t vertexId = weight.mVertexId;
				const aiVector3D& srcPos = mesh->mVertices[vertexId];
                
				modelMeshes[i].animatedPos[vertexId] += weight.mWeight * (posTrafo * srcPos);
			}
			if(mesh->HasNormals()){
				// 3x3 matrix, contains the bone matrix without the translation, only with rotation and possibly scaling
				aiMatrix3x3 normTrafo = aiMatrix3x3( posTrafo);
				for( size_t b = 0; b < bone->mNumWeights; ++b)
				{
					const aiVertexWeight& weight = bone->mWeights[b];
					size_t vertexId = weight.mVertexId;
                    
					const aiVector3D& srcNorm = mesh->mNormals[vertexId];
					modelMeshes[i].animatedNorm[vertexId] += weight.mWeight * (normTrafo * srcNorm);
                    
				}
			}
		}
	}
}