#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiFlashMovieManager.h"

namespace Maki
{
	namespace Framework
	{

		FlashMovieManager::FlashMovieManager(uint32 size)
			: Manager<FlashMovie, FlashMovieManager>(size, "FlashMovieManager")
		{
			
		}

		FlashMovieManager::~FlashMovieManager()
		{
		}

		Handle FlashMovieManager::Load(Rid rid)
		{
			Handle handle = resPool->Match(Resource::FindPredicate<FlashMovie>(rid)) | managerId;
			if(handle != HANDLE_NONE) {
				return handle;
			}

			handle = resPool->Alloc() | managerId;
			FlashMovie *m = resPool->Get(handle & HANDLE_VALUE_MASK);
			new(m) FlashMovie();
			if(!m->Load(rid)) {
				resPool->Free(handle & HANDLE_VALUE_MASK);
				return HANDLE_NONE;
			}
			return handle;
		}

		void FlashMovieManager::ReloadAssets()
		{
			Engine *eng = Engine::Get();

			const ResourcePool<FlashMovie>::Iterator end = resPool->End();
			for(ResourcePool<FlashMovie>::Iterator iter = resPool->Begin(); iter != end; ++iter) {
				FlashMovie *m = iter.Ptr();
				Rid rid = m->rid;
				if(rid != RID_NONE) {
					m->~FlashMovie();
					new(m) FlashMovie();
					m->Load(rid);
				}
			}
		}
		
		bool FlashMovieManager::ReloadAsset(Rid rid)
		{
			Handle handle = resPool->Match(Resource::FindPredicate<FlashMovie>(rid)) | managerId;
			if(handle == HANDLE_NONE) {
				return false;
			}
			FlashMovie *m = resPool->Get(handle & HANDLE_VALUE_MASK);
			resPool->Free(handle & HANDLE_VALUE_MASK);

			if(rid != RID_NONE) {
				m->~FlashMovie();
				new(m) FlashMovie();
				return m->Load(rid);
			}
			return true;
		}
		

	} // namespace Core

} // namespace Maki