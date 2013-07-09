#include "core/core_stdafx.h"
#include "core/MakiSkeletonManager.h"
#include "core/MakiSkeleton.h"
#include "core/MakiEngine.h"

namespace Maki
{
	namespace Core
	{

		SkeletonManager::SkeletonManager(uint32 size)
			: Manager<Skeleton, SkeletonManager>(size, "SkeletonManager")
		{
		}
	
		SkeletonManager::~SkeletonManager()
		{
		}
	
		Handle SkeletonManager::Load(Rid rid)
		{
			Handle handle = resPool->Match(Resource::FindPredicate<Skeleton>(rid)) | managerId;
			if(handle != HANDLE_NONE) {
				return handle;
			}

			handle = resPool->Alloc() | managerId;
			Skeleton *skel = resPool->Get(handle & HANDLE_VALUE_MASK);
			new(skel) Skeleton();
		
			if(!skel->Load(rid)) {
				resPool->Free(handle & HANDLE_VALUE_MASK);
				return HANDLE_NONE;
			}
			return handle;
		}

		void SkeletonManager::ReloadAssets()
		{
			Engine *eng = Engine::Get();

			const ResourcePool<Skeleton>::Iterator end = resPool->End();
			for(ResourcePool<Skeleton>::Iterator iter = resPool->Begin(); iter != end; ++iter) {
				Skeleton *skel = iter.Ptr();
				Rid rid = skel->rid;
				if(rid != RID_NONE) {
					skel->~Skeleton();
					new(skel) Skeleton();
					skel->Load(rid);
				}
			}
		}

		bool SkeletonManager::ReloadAsset(Rid rid)
		{
			Handle handle = resPool->Match(Resource::FindPredicate<Skeleton>(rid)) | managerId;
			if(handle == HANDLE_NONE) {
				return false;
			}
			Skeleton *skel = resPool->Get(handle & HANDLE_VALUE_MASK);
			resPool->Free(handle & HANDLE_VALUE_MASK);

			if(rid != RID_NONE) {
				skel->~Skeleton();
				new(skel) Skeleton();
				return skel->Load(rid);
			}
			return true;
		}

	} // namespace Core

} // namespace Maki