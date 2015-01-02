#include "core/core_stdafx.h"
#include "core/MakiTextureSetManager.h"

namespace maki
{
	namespace core
	{

		texture_set_manager_t::texture_set_manager_t(uint32 size)
			: manager_t<texture_set_t, texture_set_manager_t>(size, "texture_set_manager_t")
		{
		}
	
		texture_set_manager_t::~texture_set_manager_t()
		{
		}
	
		handle_t texture_set_manager_t::load(uint8 count, rid_t *texture_rids)
		{
			predicate_t p;
			p.count_ = count;
			p.texture_rids_ = texture_rids;
			handle_t handle_ = res_pool_->match(p) | manager_id_;
			if(handle_ != HANDLE_NONE) {
				return handle_;
			}

			handle_ = res_pool_->alloc() | manager_id_;
			texture_set_t *ts = res_pool_->get(handle_ & handle_value_mask_);
			new(ts) texture_set_t();
			if(!ts->load(count, texture_rids)) {
				res_pool_->free(handle_ & handle_value_mask_);
				return HANDLE_NONE;
			}
			return handle_;
		}


	} // namespace core

} // namespace maki
