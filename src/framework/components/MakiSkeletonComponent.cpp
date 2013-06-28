#pragma once
#include "framework/framework_stdafx.h"
//#include "framework/MakiDebugArmature.h"
#include "framework/MakiSystem.h"
#include "framework/components/MakiSkeletonComponent.h"
#include "framework/components/MakiMeshComponent.h"


namespace Maki
{
	namespace Framework
	{
		namespace Components
		{

			Skeleton::Skeleton()
				: Component(TYPE, DEPENDENCIES),
				skeleton(HANDLE_NONE),
				materialSlot(-1),
				poseDirty(true)
			{
			}

			Skeleton::~Skeleton()
			{
				SkeletonManager::Free(skeleton);
			}

			bool Skeleton::Init(Document::Node *node)
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

			bool Skeleton::Init(HandleOrRid skelId)
			{
				ResourceProvider *res = ResourceProvider::Get();

				if(skelId.isHandle) {
					SkeletonManager::AddRef(skelId.handle);
					skeleton = skelId.handle;
				} else {
					skeleton = res->skeletonManager->Load(skelId.rid);
					assert(skeleton != HANDLE_NONE);
				}

				// Make our buffers the right size for this skeleton
				Core::Skeleton *skel = SkeletonManager::Get(skeleton);
				pose.SetSize(skel->bones.count);
				matrixPose.SetSize(skel->bones.count);
				return true;
			}

			void Skeleton::OnAttach()
			{
				// Skinned meshes need exclusive access to their material (since they must write matrices to it)
				Components::Mesh *meshComp = owner->Get<Components::Mesh>();
				meshComp->material = MaterialManager::DuplicateIfShared(meshComp->material);

				// Setup the material constant slot that will receive the bone matrices
				// Bone matrices are 3x4 = 12 floats = 12*4 bytes
				Core::Skeleton *skel = SkeletonManager::Get(skeleton);
				Material *mat = MaterialManager::Get(meshComp->material);
				char *buffer = (char *)Allocator::Malloc(skel->bones.count*12*4);
				materialSlot = mat->PushConstant("uBoneStates", skel->bones.count*12*4, buffer);
			}

		} // namespace Components

	} // namespace Framework

} // namespace Maki