#include "core/MakiVertexFormat.h"
#include "core/MakiEngine.h"

namespace maki {
	namespace core {

		const uint8_t vertex_format_t::data_type_sizes[data_type_count] = {4, 4, 2, 1};

		void vertex_format_t::push_attribute(attribute_t attr, data_type_t type, uint8_t count) {
			MAKI_ASSERT(count <= 64);
			attr_flags_ |= 1 << attr;
			formats_[attr].type = type;
			formats_[attr].count = count;
			stride_ += data_type_sizes[type] * count;
			attr_count_++;
		}

	} // namespace core
} // namespace maki
