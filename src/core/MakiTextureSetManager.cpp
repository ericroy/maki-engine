#include "core/MakiTextureSetManager.h"

namespace maki {
	namespace core {

		texture_set_manager_t::texture_set_manager_t(uint32_t capacity) {
			res_pool_.reset(new resource_pool_t<texture_set_t>(capacity));
		}

		ref_t<texture_set_t> texture_set_manager_t::get(uint8_t count, rid_t *rids) {
			return res_pool_->find([&rids, count](const texture_set_t &ts) {
				if (count != ts.texture_count)
					return false;
				for (uint8_t i = 0; i < count; i++) {
					if (rids[i] != ts.texture_rids[i] || rids[i] == RID_NONE)
						return false;
				}
				return true;
			});
		}

		ref_t<texture_set_t> texture_set_manager_t::load(uint8_t count, rid_t *rids) {
			auto ts = res_pool_->alloc();
			return ts->load(count, rids) ? ts : nullptr;
		}

		ref_t<texture_set_t> texture_set_manager_t::get_or_load(uint8_t count, rid_t *rids) {
			auto ts = get(count, rids);
			return ts ? ts : load(count, rids);
		}


	} // namespace core
} // namespace maki
