#include "core/core_stdafx.h"
#include "core/MakiResource.h"

namespace Maki
{
		
	Resource::Resource(const Movable<Resource> &other)
	{
		rid = other.obj->rid;
		other.obj->rid = RID_NONE;
	}

} // namespace Maki