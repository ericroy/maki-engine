#include "core/MakiResource.h"

namespace maki {
	namespace core {
		
		resource_t::resource_t(resource_t &&other) {
			rid_ = other.rid_;
			other.rid_ = RID_NONE;
		}

	} // namespace core
} // namespace maki
