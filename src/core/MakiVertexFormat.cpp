#include "core/core_stdafx.h"
#include "core/MakiVertexFormat.h"
#include "core/MakiEngine.h"

namespace maki
{
	namespace core
	{

		const uint8 vertex_format_t::DataTypeSizes[DataTypeCount] = {4, 4, 2, 1};

		vertex_format_t::vertex_format_t()
			: stride(0),
			attrCount(0),
			attrFlags(0)
		{
			memset(formats, 0, sizeof(formats));
		}

		vertex_format_t::vertex_format_t(const vertex_format_t &other)
			: stride(other.stride),
			attrCount(other.attrCount),
			attrFlags(other.attrFlags)
		{
			memcpy(formats, other.formats, sizeof(formats));
		}

		vertex_format_t::~vertex_format_t() {
		}

		void vertex_format_t::PushAttribute(Attribute attr, DataType type, uint8 count)
		{
			assert(count <= 64);
			attrFlags |= 1<<attr;
			formats[attr].type_ = type;
			formats[attr].count_ = count;
			stride += DataTypeSizes[type]*count;
			attrCount++;
		}

	} // namespace core

} // namespace maki
