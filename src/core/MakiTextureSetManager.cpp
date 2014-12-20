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
	
		handle_t texture_set_manager_t::load(uint8 count, rid_t *textureRids)
		{
			predicate_t p;
			p.count_ = count;
			p.textureRids = textureRids;
			handle_t handle = resPool->Match(p) | managerId;
			if(handle != HANDLE_NONE) {
				return handle;
			}

			handle = resPool->alloc() | managerId;
			texture_set_t *ts = resPool->get(handle & handle_value_mask_);
			new(ts) texture_set_t();
			if(!ts->load(count, textureRids)) {
				resPool->free(handle & handle_value_mask_);
				return HANDLE_NONE;
			}
			return handle;
		}


	} // namespace core

} // namespace maki
