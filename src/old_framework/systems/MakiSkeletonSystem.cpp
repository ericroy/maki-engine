#include "framework/framework_stdafx.h"
#include "framework/systems/MakiSkeletonSystem.h"
#include "framework/components/MakiMeshComponent.h"
#include "framework/components/MakiSkeletonComponent.h"


namespace Maki
{
	namespace Framework
	{
		namespace Systems
		{

			SkeletonSystem::SkeletonSystem(uint32 messageQueueSize)
				: System(Component::TypeFlag_Skeleton|Component::TypeFlag_Mesh, 0, messageQueueSize, "SkeletonSystem")
			{
			}

			SkeletonSystem::~SkeletonSystem()
			{
			}

			void SkeletonSystem::Update()
			{
				const uint32 count = nodes.size();
				for(uint32 i = 0; i < count; i++) {
					Components::Skeleton *skelComp = std::get<0>(nodes[i]);
					Components::Mesh *meshComp = std::get<1>(nodes[i]);
			
					Material *mat = MaterialManager::Get(meshComp->material);
			
					assert(skelComp->skeleton != HANDLE_NONE);

					if(!skelComp->poseDirty) {
						return;
					}
					skelComp->poseDirty = false;

					Skeleton *skel = SkeletonManager::Get(skelComp->skeleton);

					skel->CalculateWorldPose(skelComp->pose.data, skelComp->matrixPose.data);
					/*if(item.skelComp->armature != nullptr) {
						item.skelComp->armature->Update(skel, item.skelComp->pose.data);
					}*/

					// Combine inverse bind and world matrices
					// Store them in the material constant buffer (as 3x4 matrices)
					float *boneMatrices = (float *)mat->uniformValues[skelComp->materialSlot].data;

					Matrix44 temp;
					for(uint32 i = 0; i < skelComp->pose.count; i++) {
						temp = skelComp->matrixPose[i] * skel->inverseBindPose[i];
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
			
		} // namespace Systems

	} // namespace Framework

} // namespace Maki