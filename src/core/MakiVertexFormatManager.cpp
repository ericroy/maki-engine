#include "core/core_stdafx.h"
#include "core/MakiVertexFormatManager.h"
#include "core/MakiEngine.h"
#include "core/MakiDrawCommand.h"

namespace Maki
{
	namespace Core
	{

		template class Manager<VertexFormat, VertexFormatManager>;

		VertexFormatManager::VertexFormatManager(uint32 size)
			: Manager<VertexFormat, VertexFormatManager>(size, "VertexFormatManager")
		{
			assert(size <= (1<<DrawCommand::BITS_PER_VERTEX_FORMAT) && "VertexFormatManager too large, add more bits in DrawCommand");
		}
	
		VertexFormatManager::~VertexFormatManager()
		{
		}


	} // namespace Core

} // namespace Maki