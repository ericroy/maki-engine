#include "core/MakiVertexFormatManager.h"

namespace maki {
	namespace core {

		vertex_format_manager_t::vertex_format_manager_t(uint32_t capacity) {
			res_pool_.reset(new resource_pool_t<vertex_format_t>(capacity, "vertex_format_manager_t"));
		}

	} // namespace core
} // namespace maki
