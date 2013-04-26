#include "core/core_stdafx.h"
#include "core/MakiResource.h"

namespace Maki
{
		
	Resource::Resource(Resource &&other)
	{
		rid = other.rid;
		other.rid = RID_NONE;
	}

} // namespace Maki