#include "core/MakiSkeletonAnimationManager.h"
#include "core/MakiSkeletonAnimation.h"
#include "core/MakiEngine.h"

namespace maki {
	namespace core {

		skeleton_animation_manager_t::skeleton_animation_manager_t(uint64_t capacity)
			: manager_t<skeleton_animation_t, skeleton_animation_manager_t>(capacity, "skeleton_animation_manager_t") {
		}
	
		handle_t skeleton_animation_manager_t::load(rid_t rid) {
			handle_t handle = res_pool_->match(resource_t::find_predicate_t<skeleton_animation_t>(rid)) | manager_id_;
			if(handle != HANDLE_NONE)
				return handle;

			handle = res_pool_->alloc() | manager_id_;
			skeleton_animation_t *skel_anim = res_pool_->get(handle & handle_value_mask_);
			new(skel_anim) skeleton_animation_t();
		
			if(!skel_anim->load(rid)) {
				res_pool_->free(handle & handle_value_mask_);
				return HANDLE_NONE;
			}
			return handle;
		}

		void skeleton_animation_manager_t::reload_assets() {
			engine_t *eng = engine_t::get();

			for(auto iter = std::begin(res_pool_); iter != std::end(res_pool_); ++iter) {
				auto *skel_anim = iter.ptr();
				auto rid = skel_anim->rid;
				if(rid != RID_NONE) {
					skel_anim->~skeleton_animation_t();
					new(skel_anim) skeleton_animation_t();
					skel_anim->load(rid);
				}
			}
		}
		
		bool skeleton_animation_manager_t::reload_asset(rid_t rid) {
			handle_t handle = res_pool_->match(resource_t::find_predicate_t<skeleton_animation_t>(rid)) | manager_id_;
			if(handle == HANDLE_NONE)
				return false;

			auto *skel_anim = res_pool_->get(handle & handle_value_mask_);
			res_pool_->free(handle & handle_value_mask_);

			if(rid != RID_NONE) {
				skel_anim->~skeleton_animation_t();
				new(skel_anim) skeleton_animation_t();
				return skel_anim->load(rid);
			}
			return true;
		}


	} // namespace core
} // namespace maki
