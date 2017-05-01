#include "core/MakiTextureSetManager.h"

namespace maki {
	namespace core {

		texture_set_manager_t::texture_set_manager_t(uint64_t capacity)
			: manager_t<texture_set_t, texture_set_manager_t>(capacity, "texture_set_manager_t") {
		}

		handle_t texture_set_manager_t::load(uint8_t count, rid_t *texture_rids) {
			predicate_t p;
			p.count = count;
			p.texture_rids = texture_rids;
			handle_t handle = res_pool_->match(p) | manager_id_;
			if(handle != HANDLE_NONE)
				return handle;

			handle = res_pool_->alloc() | manager_id_;
			texture_set_t *ts = res_pool_->get(handle & handle_value_mask_);
			new(ts) texture_set_t();
			if(!ts->load(count, texture_rids)) {
				res_pool_->free(handle & handle_value_mask_);
				return HANDLE_NONE;
			}
			return handle;
		}


	} // namespace core
} // namespace maki
