#include "core/core_stdafx.h"
#include "core/MakiSkeletonAnimationManager.h"
#include "core/MakiSkeletonAnimation.h"
#include "core/MakiEngine.h"

namespace maki
{
	namespace core
	{

		skeleton_animation_manager_t::skeleton_animation_manager_t(uint32 size)
			: manager_t<skeleton_animation_t, skeleton_animation_manager_t>(size, "skeleton_animation_manager_t")
		{
		}
	
		skeleton_animation_manager_t::~skeleton_animation_manager_t()
		{
		}
	
		handle_t skeleton_animation_manager_t::load(rid_t rid)
		{
			handle_t handle = resPool->Match(resource_t::find_predicate_t<skeleton_animation_t>(rid)) | managerId;
			if(handle != HANDLE_NONE) {
				return handle;
			}

			handle = resPool->alloc() | managerId;
			skeleton_animation_t *skelAnim = resPool->get(handle & handle_value_mask_);
			new(skelAnim) skeleton_animation_t();
		
			if(!skelAnim->load(rid)) {
				resPool->free(handle & handle_value_mask_);
				return HANDLE_NONE;
			}
			return handle;
		}

		void skeleton_animation_manager_t::reload_assets()
		{
			engine_t *eng = engine_t::get();

			const resource_pool_t<skeleton_animation_t>::iterator_t end = resPool->end();
			for(resource_pool_t<skeleton_animation_t>::iterator_t iter = resPool->begin(); iter != end; ++iter) {
				skeleton_animation_t *skelAnim = iter.Ptr();
				rid_t rid = skelAnim->rid_;
				if(rid != RID_NONE) {
					skelAnim->~skeleton_animation_t();
					new(skelAnim) skeleton_animation_t();
					skelAnim->load(rid);
				}
			}
		}
		
		bool skeleton_animation_manager_t::reload_asset(rid_t rid)
		{
			handle_t handle = resPool->Match(resource_t::find_predicate_t<skeleton_animation_t>(rid)) | managerId;
			if(handle == HANDLE_NONE) {
				return false;
			}
			skeleton_animation_t *skelAnim = resPool->get(handle & handle_value_mask_);
			resPool->free(handle & handle_value_mask_);

			if(rid != RID_NONE) {
				skelAnim->~skeleton_animation_t();
				new(skelAnim) skeleton_animation_t();
				return skelAnim->load(rid);
			}
			return true;
		}


	} // namespace core

} // namespace maki
