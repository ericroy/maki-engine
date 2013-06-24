#pragma once
#include "framework/framework_stdafx.h"
//#include "framework/MakiDebugArmature.h"
#include "framework/systems/MakiSkeletonSystem.h"
#include "framework/components/MakiMeshComponent.h"
#include "framework/components/MakiSkeletonComponent.h"

namespace Maki
{
	namespace Framework
	{

		SkeletonSystem::SkeletonSystem()
			: System(Component::Type_Mesh|Component::Type_Skeleton)
		{
		}

		SkeletonSystem::~SkeletonSystem()
		{
		}

		void SkeletonSystem::Update()
		{
			const uint32 count = items.size();
			for(uint32 i = 0; i < count; i++) {
				const Item &item = items[i];
			
				Material *mat = MaterialManager::Get(item.meshComp->material);
			
				assert(item.skelComp->skeleton != HANDLE_NONE);

				if(!item.skelComp->poseDirty) {
					return;
				}
				item.skelComp->poseDirty = false;

				Skeleton *skel = SkeletonManager::Get(item.skelComp->skeleton);

				skel->CalculateWorldPose(item.skelComp->pose.data, item.skelComp->matrixPose.data);
				/*if(item.skelComp->armature != nullptr) {
					item.skelComp->armature->Update(skel, item.skelComp->pose.data);
				}*/

				// Combine inverse bind and world matrices
				// Store them in the material constant buffer (as 3x4 matrices)
				float *boneMatrices = (float *)mat->uniformValues[item.skelComp->materialSlot].data;

				Matrix44 temp;
				for(uint32 i = 0; i < item.skelComp->pose.count; i++) {
					temp = item.skelComp->matrixPose[i] * skel->inverseBindPose[i];
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
		}

		void SkeletonSystem::Add(Entity *e)
		{
			Item item;
			item.meshComp = e->Get<Components::Mesh>();
			item.skelComp = e->Get<Components::Skeleton>();
			items.push_back(item);
		}

		void SkeletonSystem::Remove(Entity *e)
		{
			Item item;
			item.meshComp = e->Get<Components::Mesh>();
			item.skelComp = e->Get<Components::Skeleton>();
			items.erase(std::find(std::begin(items), std::end(items), item));
		}


	} // namespace Framework

} // namespace Maki