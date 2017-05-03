#include "core/MakiVertexFormat.h"

namespace maki {
	namespace core {

		void vertex_format_t::push_attribute(attribute_t attr, attribute_type_t type, uint8_t element_count) {
			MAKI_ASSERT(element_count <= 64);
			attribute_flags_ |= 1 << attr;
			attribute_descriptors_[attr].type = type;
			attribute_descriptors_[attr].element_count = element_count;
			stride_ += attribute_type_sizes[type] * element_count;
			attribute_count_++;
		}

	} // namespace core
} // namespace maki
