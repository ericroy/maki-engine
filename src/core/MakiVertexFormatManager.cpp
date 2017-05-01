#include "core/MakiVertexFormatManager.h"
#include "core/MakiEngine.h"
#include "core/MakiDrawCommand.h"

namespace maki {
	namespace core {

		template class manager_t<vertex_format_t, vertex_format_manager_t>;

		vertex_format_manager_t::vertex_format_manager_t(uint64_t capacity)
			: manager_t<vertex_format_t, vertex_format_manager_t>(capacity, "vertex_format_manager_t") {
			assert(capacity <= (1 << draw_command_t::bits_per_vertex_format) && "vertex_format_manager_t too large, add more bits in draw_command_t");
		}

	} // namespace core
} // namespace maki
