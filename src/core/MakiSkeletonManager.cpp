#include "core/MakiSkeletonManager.h"

namespace maki {
	namespace core {

		skeleton_manager_t::skeleton_manager_t(uint32_t capacity) : res_pool_(capacity, "skeleton_manager_t") {
		}

		ref_t<skeleton_t> skeleton_manager_t::get(rid_t rid) {
			return res_pool_.find([rid](const skeleton_t &m) {
				return rid == m.rid;
			});
		}

		ref_t<skeleton_t> skeleton_manager_t::load(rid_t rid) {
			auto sk = res_pool_.alloc();
			return sk->load(rid) ? sk : nullptr;
		}

		ref_t<skeleton_t> skeleton_manager_t::get_or_load(rid_t rid) {
			auto sk = get(rid);
			return sk ? sk : load(rid);
		}

	} // namespace core
} // namespace maki
