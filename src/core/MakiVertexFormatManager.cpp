#include "core/MakiVertexFormatManager.h"
#include "core/MakiDrawCommand.h"

namespace maki {
	namespace core {

		vertex_format_manager_t::vertex_format_manager_t(uint32_t capacity) : res_pool_(capacity, "vertex_format_manager_t") {
			MAKI_ASSERT(capacity <= (1 << draw_command_t::bits_per_vertex_format) && "assign more bits to vertex format in draw command bitfield");
		}

	} // namespace core
} // namespace maki
