#include "core/core_stdafx.h"
#include "core/MakiVertexFormat.h"
#include "core/MakiEngine.h"

namespace Maki
{

	const uint8 VertexFormat::DataTypeSizes[DataTypeCount] = {4, 4, 2, 1};

	VertexFormat::VertexFormat()
	:	stride(0),
		attrCount(0),
		attrFlags(0)
	{
		memset(formats, 0, sizeof(formats));
	}

	VertexFormat::~VertexFormat() {
	}

} // namespace Maki