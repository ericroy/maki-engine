#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiDebugArmature.h"
#include "framework/MakiSystem.h"
#include "framework/components/MakiSkeletonComponent.h"
#include "framework/components/MakiMeshComponent.h"


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
			System::PostMessage(this, Message_DebugWidgetCreated, (Entity *)armature);
		}
		
		return true;
	}


} // namespace Maki