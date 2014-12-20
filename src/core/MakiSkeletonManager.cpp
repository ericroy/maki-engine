#include "core/core_stdafx.h"
#include "core/MakiSkeletonManager.h"
#include "core/MakiSkeleton.h"
#include "core/MakiEngine.h"

namespace maki
{
	namespace core
	{

		skeleton_manager_t::skeleton_manager_t(uint32 size)
			: manager_t<skeleton_t, skeleton_manager_t>(size, "skeleton_manager_t")
		{
		}
	
		skeleton_manager_t::~skeleton_manager_t()
		{
		}
	
		handle_t skeleton_manager_t::load(rid_t rid)
		{
			handle_t handle = resPool->Match(resource_t::find_predicate_t<skeleton_t>(rid)) | managerId;
			if(handle != HANDLE_NONE) {
				return handle;
			}

			handle = resPool->alloc() | managerId;
			skeleton_t *skel = resPool->get(handle & handle_value_mask_);
			new(skel) skeleton_t();
		
			if(!skel->load(rid)) {
				resPool->free(handle & handle_value_mask_);
				return HANDLE_NONE;
			}
			return handle;
		}

		void skeleton_manager_t::reload_assets()
		{
			engine_t *eng = engine_t::get();

			const resource_pool_t<skeleton_t>::iterator_t end = resPool->end();
			for(resource_pool_t<skeleton_t>::iterator_t iter = resPool->begin(); iter != end; ++iter) {
				skeleton_t *skel = iter.Ptr();
				rid_t rid = skel->rid_;
				if(rid != RID_NONE) {
					skel->~skeleton_t();
					new(skel) skeleton_t();
					skel->load(rid);
				}
			}
		}

		bool skeleton_manager_t::reload_asset(rid_t rid)
		{
			handle_t handle = resPool->Match(resource_t::find_predicate_t<skeleton_t>(rid)) | managerId;
			if(handle == HANDLE_NONE) {
				return false;
			}
			skeleton_t *skel = resPool->get(handle & handle_value_mask_);
			resPool->free(handle & handle_value_mask_);

			if(rid != RID_NONE) {
				skel->~skeleton_t();
				new(skel) skeleton_t();
				return skel->load(rid);
			}
			return true;
		}

	} // namespace core

} // namespace maki
