#include "core/core_stdafx.h"
#include "core/MakiSkeletonAnimationManager.h"
#include "core/MakiSkeletonAnimation.h"
#include "core/MakiEngine.h"

namespace Maki
{
	namespace Core
	{

		SkeletonAnimationManager::SkeletonAnimationManager(uint32 size)
			: Manager<SkeletonAnimation, SkeletonAnimationManager>(size)
		{
		}
	
		SkeletonAnimationManager::~SkeletonAnimationManager()
		{
		}
	
		Handle SkeletonAnimationManager::Load(Rid rid)
		{
			Handle handle = resPool->Match(Resource::FindPredicate<SkeletonAnimation>(rid)) | managerId;
			if(handle != HANDLE_NONE) {
				return handle;
			}

			handle = resPool->Alloc() | managerId;
			SkeletonAnimation *skelAnim = resPool->Get(handle & HANDLE_VALUE_MASK);
			new(skelAnim) SkeletonAnimation();
		
			if(!skelAnim->Load(rid)) {
				resPool->Free(handle & HANDLE_VALUE_MASK);
				return HANDLE_NONE;
			}
			return handle;
		}

		void SkeletonAnimationManager::ReloadAssets()
		{
			Engine *eng = Engine::Get();

			const ResourcePool<SkeletonAnimation>::Iterator end = resPool->End();
			for(ResourcePool<SkeletonAnimation>::Iterator iter = resPool->Begin(); iter != end; ++iter) {
				SkeletonAnimation *skelAnim = iter.Ptr();
				Rid rid = skelAnim->rid;
				if(rid != RID_NONE) {
					skelAnim->~SkeletonAnimation();
					new(skelAnim) SkeletonAnimation();
					skelAnim->Load(rid);
				}
			}
		}
		
		bool SkeletonAnimationManager::ReloadAsset(Rid rid)
		{
			Handle handle = resPool->Match(Resource::FindPredicate<SkeletonAnimation>(rid)) | managerId;
			if(handle == HANDLE_NONE) {
				return false;
			}
			SkeletonAnimation *skelAnim = resPool->Get(handle & HANDLE_VALUE_MASK);
			resPool->Free(handle & HANDLE_VALUE_MASK);

			if(rid != RID_NONE) {
				skelAnim->~SkeletonAnimation();
				new(skelAnim) SkeletonAnimation();
				return skelAnim->Load(rid);
			}
			return true;
		}


	} // namespace Core

} // namespace Maki