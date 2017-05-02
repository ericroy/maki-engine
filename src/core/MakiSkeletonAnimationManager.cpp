#include "core/MakiSkeletonAnimationManager.h"
#include "core/MakiSkeletonAnimation.h"
#include "core/MakiEngine.h"

namespace maki {
	namespace core {

		skeleton_animation_manager_t::skeleton_animation_manager_t(uint32_t capacity): res_pool_(capacity, "skeleton_animation_manager_t") {
		}

		ref_t<skeleton_animation_t> skeleton_animation_manager_t::get(rid_t rid) {
			return res_pool_->find([rid](const skeleton_animation_t &ska) {
				return rid == ska.rid();
			});
		}

		ref_t<skeleton_animation_t> skeleton_animation_manager_t::load(rid_t rid) {
			auto ska = res_pool_->alloc();
			return ska->load(rid) ? ska : nullptr;
		}

		ref_t<skeleton_animation_t> skeleton_animation_manager_t::get_or_load(rid_t rid) {
			auto ska = get(rid);
			return ska ? ska : load(rid);
		}

	} // namespace core
} // namespace maki
