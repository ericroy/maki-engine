#include "core/MakiResource.h"

namespace maki {
	namespace core {
		
		resource_t::resource_t(resource_t &&other) {
			rid_ = other.rid_;
			other.rid_ = RID_NONE;
		}

		resource_t::resource_t(const resource_t &other) {
			rid_ = other.rid_;
		}

	} // namespace core
} // namespace maki
