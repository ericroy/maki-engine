#include "core/core_stdafx.h"
#include "core/MakiVertexFormatManager.h"
#include "core/MakiEngine.h"
#include "core/MakiDrawCommand.h"

namespace maki
{
	namespace core
	{

		template class manager_t<vertex_format_t, vertex_format_manager_t>;

		vertex_format_manager_t::vertex_format_manager_t(uint32_t size)
			: manager_t<vertex_format_t, vertex_format_manager_t>(size, "vertex_format_manager_t")
		{
			assert(size <= (1<<draw_command_t::bits_per_vertex_format_) && "vertex_format_manager_t too large, add more bits in draw_command_t");
		}
	
		vertex_format_manager_t::~vertex_format_manager_t()
		{
		}


	} // namespace core

} // namespace maki
