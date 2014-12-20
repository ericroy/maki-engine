#include "core/core_stdafx.h"
#include "core/MakiResource.h"

namespace maki
{
	namespace core
	{
		
		resource_t::resource_t(const move_token_t<resource_t> &other)
		{
			rid = other.obj->rid_;
			other.obj->rid_ = RID_NONE;
		}

	} // namespace core

} // namespace maki
