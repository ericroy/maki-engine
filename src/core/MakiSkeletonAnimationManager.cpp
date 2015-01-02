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
			handle_t handle_ = res_pool_->match(resource_t::find_predicate_t<skeleton_animation_t>(rid)) | manager_id_;
			if(handle_ != HANDLE_NONE) {
				return handle_;
			}

			handle_ = res_pool_->alloc() | manager_id_;
			skeleton_animation_t *skel_anim = res_pool_->get(handle_ & handle_value_mask_);
			new(skel_anim) skeleton_animation_t();
		
			if(!skel_anim->load(rid)) {
				res_pool_->free(handle_ & handle_value_mask_);
				return HANDLE_NONE;
			}
			return handle_;
		}

		void skeleton_animation_manager_t::reload_assets()
		{
			engine_t *eng = engine_t::get();

			const resource_pool_t<skeleton_animation_t>::iterator_t end = res_pool_->end();
			for(resource_pool_t<skeleton_animation_t>::iterator_t iter = res_pool_->begin(); iter != end; ++iter) {
				skeleton_animation_t *skel_anim = iter.ptr();
				rid_t rid = skel_anim->rid_;
				if(rid != RID_NONE) {
					skel_anim->~skeleton_animation_t();
					new(skel_anim) skeleton_animation_t();
					skel_anim->load(rid);
				}
			}
		}
		
		bool skeleton_animation_manager_t::reload_asset(rid_t rid)
		{
			handle_t handle_ = res_pool_->match(resource_t::find_predicate_t<skeleton_animation_t>(rid)) | manager_id_;
			if(handle_ == HANDLE_NONE) {
				return false;
			}
			skeleton_animation_t *skel_anim = res_pool_->get(handle_ & handle_value_mask_);
			res_pool_->free(handle_ & handle_value_mask_);

			if(rid != RID_NONE) {
				skel_anim->~skeleton_animation_t();
				new(skel_anim) skeleton_animation_t();
				return skel_anim->load(rid);
			}
			return true;
		}


	} // namespace core

} // namespace maki
