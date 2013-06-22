#pragma once
#include "framework/framework_stdafx.h"
#include "framework/components/MakiSkeletonComponent.h"
#include "framework/components/MakiMeshComponent.h"
#include "framework/MakiDebugArmature.h"

namespace Maki
{

	std::function<Entity *()> SkeletonComponent::CreateDebugArmature = nullptr;

	SkeletonComponent::SkeletonComponent()
		: Component(COMPONENT_TYPE),
		skeleton(HANDLE_NONE),
		armature(nullptr),
		materialSlot(-1),
		poseDirty(true)
	{
	}

	SkeletonComponent::SkeletonComponent(HandleOrRid skeletonId)
		: Component(COMPONENT_TYPE),
		skeleton(HANDLE_NONE),
		armature(nullptr),
		materialSlot(-1),
		poseDirty(true)
	{
		bool ret = Init(skeletonId);
		assert(ret);
	}

	SkeletonComponent::~SkeletonComponent()
	{
		SkeletonManager::Free(skeleton);
	}

	bool SkeletonComponent::Init(Document::Node *node)
	{
		Engine *eng = Engine::Get();

		Rid skelRid = RID_NONE;
		const char *skelPath = node->ResolveValue("skeleton.#0");
		if(skelPath != nullptr) {
			skelRid = Engine::Get()->assets->PathToRid(skelPath);
		}
		if(skelRid == RID_NONE) {
			Console::Error("No RID for path: %s", skelRid);
			return false;
		}

		return Init(skelRid);
	}

	void SkeletonComponent::Update(Material *mat)
	{
		if(skeleton == HANDLE_NONE) {
			return;
		}

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

	bool SkeletonComponent::Init(HandleOrRid skelId)
	{
		ResourceProvider *res = ResourceProvider::Get();

		if(skelId.isHandle) {
			SkeletonManager::AddRef(skelId.handle);
			skeleton = skelId.handle;
		} else {
			skeleton = res->skeletonManager->Load(skelId.rid);
			assert(skeleton != HANDLE_NONE);
		}

		// Skinned meshes need exclusive access to their material (since they must write matrices to it)
		MeshComponent *meshComp = owner->Get<MeshComponent>();
		meshComp->material = MaterialManager::DuplicateIfShared(meshComp->material);

		// Make our buffers the right size for this skeleton
		Skeleton *skel = SkeletonManager::Get(skeleton);
		pose.SetSize(skel->bones.count);
		matrixPose.SetSize(skel->bones.count);

		// Setup the material constant slot that will receive the bone matrices
		// Bone matrices are 3x4 = 12 floats = 12*4 bytes
		Material *mat = MaterialManager::Get(meshComp->material);
		char *buffer = (char *)Allocator::Malloc(skel->bones.count*12*4);
		materialSlot = mat->PushConstant("uBoneStates", skel->bones.count*12*4, buffer);
		
		if(CreateDebugArmature != nullptr) {
			armature = (DebugArmature *)CreateDebugArmature();
			owner->SendMessage(this, Message_DebugWidgetCreated, (Entity *)armature, nullptr);
		}
		
		return true;
	}


} // namespace Maki