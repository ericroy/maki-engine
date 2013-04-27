#include "framework/framework_stdafx.h"
#include "framework/MakiSkinnedMeshEntity.h"

namespace Maki
{

	std::function<Entity *()> SkinnedMeshEntity::CreateDebugArmature = nullptr;


	SkinnedMeshEntity::SkinnedMeshEntity(HandleOrRid meshId, HandleOrRid matId, HandleOrRid skelId)
	:	MeshEntity(),
		skeleton(HANDLE_NONE),
		armature(nullptr),
		materialSlot(-1),
		poseDirty(true)
	{
		ResourceProvider *res = ResourceProvider::Get();
		
		Handle matHandle = HANDLE_NONE;
		if(matId.isHandle) {
			if(matId.handle != HANDLE_NONE) {
				MaterialManager::AddRef(matId.handle);
				matHandle = matId.handle;
			}
		} else {
			matHandle = res->materialManager->Load(matId.rid);
		}

		// Get exclusive ownership of our material
		matHandle = MaterialManager::DuplicateIfShared(matHandle);
		
		// Initialize the superclass and release our temp handle to the material (we'll use the one in the superclass from now on)
		MeshEntity::Init(meshId, matHandle);
		MaterialManager::Free(matHandle);


		if(skelId.isHandle) {
			SkeletonManager::AddRef(skelId.handle);
			skeleton = skelId.handle;
		} else {
			skeleton = res->skeletonManager->Load(skelId.rid);
		}
		assert(skeleton != HANDLE_NONE);
		
		// Make our buffers the right size for this skeleton
		Skeleton *skel = SkeletonManager::Get(skeleton);
		pose.SetSize(skel->bones.count);
		matrixPose.SetSize(skel->bones.count);

		// Setup the material constant slot that will receive the bone matrices
		// Bone matrices are 3x4 = 12 floats = 12*4 bytes
		Material *mat = MaterialManager::Get(material);
		char *buffer = (char *)Allocator::Malloc(skel->bones.count*12*4);
		materialSlot = mat->PushConstant("uBoneStates", skel->bones.count*12*4, buffer);
		
		if(CreateDebugArmature != nullptr) {
			armature = (DebugArmature *)CreateDebugArmature();
			AddChild(armature);
		}
	}

	SkinnedMeshEntity::~SkinnedMeshEntity() {
		SkeletonManager::Free(skeleton);
		// Don't delete the armature - it was added to the scene, and will be deleted by the scene
	}

	void SkinnedMeshEntity::Update(float dt) {
		if(!poseDirty) {
			return;
		}
		poseDirty = false;

		Skeleton *skel = SkeletonManager::Get(skeleton);

		skel->CalculateWorldPose(pose.data, matrixPose.data);
		if(armature != nullptr) {
			armature->Update(skel, pose.data);
		}

		// Combine inverse bind and world matrices
		// Store them in the material constant buffer (as 3x4 matrices)
		Material *mat = MaterialManager::Get(material);
		float *boneMatrices = (float *)mat->uniformValues[materialSlot].data;

		Matrix44 temp;
		for(uint32 i = 0; i < pose.count; i++) {
			temp = matrixPose[i] * skel->inverseBindPose[i];
			const uint32 base = i*12;
			boneMatrices[base] = temp.cols[0][0];
			boneMatrices[base+1] = temp.cols[0][1];
			boneMatrices[base+2] = temp.cols[0][2];
			boneMatrices[base+3] = temp.cols[1][0];
			boneMatrices[base+4] = temp.cols[1][1];
			boneMatrices[base+5] = temp.cols[1][2];
			boneMatrices[base+6] = temp.cols[2][0];
			boneMatrices[base+7] = temp.cols[2][1];
			boneMatrices[base+8] = temp.cols[2][2];
			boneMatrices[base+9] = temp.cols[3][0];
			boneMatrices[base+10] = temp.cols[3][1];
			boneMatrices[base+11] = temp.cols[3][2];
		}
	}

} // namespace Maki