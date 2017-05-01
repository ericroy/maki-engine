#include "core/MakiSkeletonManager.h"
#include "core/MakiSkeleton.h"
#include "core/MakiEngine.h"

namespace maki {
	namespace core {

		skeleton_manager_t::skeleton_manager_t(uint64_t capacity)
			: manager_t<skeleton_t, skeleton_manager_t>(capacity, "skeleton_manager_t") {
		}

		handle_t skeleton_manager_t::load(rid_t rid) {
			handle_t handle = res_pool_->match(resource_t::find_predicate_t<skeleton_t>(rid)) | manager_id_;
			if(handle != HANDLE_NONE)
				return handle;

			handle = res_pool_->alloc() | manager_id_;
			skeleton_t *skel = res_pool_->get(handle & handle_value_mask_);
			new(skel) skeleton_t();
		
			if(!skel->load(rid)) {
				res_pool_->free(handle & handle_value_mask_);
				return HANDLE_NONE;
			}
			return handle;
		}

		void skeleton_manager_t::reload_assets() {
			engine_t *eng = engine_t::get();

			for(auto iter = std::begin(res_pool_); iter != std::end(res_pool_); ++iter) {
				auto *skel = iter.ptr();
				auto rid = skel->rid;
				if(rid != RID_NONE) {
					skel->~skeleton_t();
					new(skel) skeleton_t();
					skel->load(rid);
				}
			}
		}

		bool skeleton_manager_t::reload_asset(rid_t rid) {
			handle_t handle = res_pool_->match(resource_t::find_predicate_t<skeleton_t>(rid)) | manager_id_;
			if(handle == HANDLE_NONE)
				return false;

			auto *skel = res_pool_->get(handle & handle_value_mask_);
			res_pool_->free(handle & handle_value_mask_);

			if(rid != RID_NONE) {
				skel->~skeleton_t();
				new(skel) skeleton_t();
				return skel->load(rid);
			}
			return true;
		}

	} // namespace core
} // namespace maki
