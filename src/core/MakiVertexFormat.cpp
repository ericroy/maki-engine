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

	void VertexFormat::PushAttribute(Attribute attr, DataType type, uint8 count)
	{
		assert(count <= 64);
		attrFlags |= 1<<attr;
		formats[attr].type = type;
		formats[attr].count = count;
		stride += DataTypeSizes[type]*count;
		attrCount++;
	}

} // namespace Maki