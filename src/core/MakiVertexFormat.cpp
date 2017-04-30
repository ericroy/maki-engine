#include "core/core_stdafx.h"
#include "core/MakiVertexFormat.h"
#include "core/MakiEngine.h"

namespace maki
{
	namespace core
	{

		const uint8_t vertex_format_t::data_type_sizes_[data_type_count_] = {4, 4, 2, 1};

		vertex_format_t::vertex_format_t()
			: stride_(0),
			attr_count_(0),
			attr_flags_(0)
		{
			memset(formats_, 0, sizeof(formats_));
		}

		vertex_format_t::vertex_format_t(const vertex_format_t &other)
			: stride_(other.stride_),
			attr_count_(other.attr_count_),
			attr_flags_(other.attr_flags_)
		{
			memcpy(formats_, other.formats_, sizeof(formats_));
		}

		vertex_format_t::~vertex_format_t() {
		}

		void vertex_format_t::push_attribute(attribute_t attr, data_type_t type, uint8_t count)
		{
			assert(count <= 64);
			attr_flags_ |= 1<<attr;
			formats_[attr].type_ = type;
			formats_[attr].count_ = count;
			stride_ += data_type_sizes_[type]*count;
			attr_count_++;
		}

	} // namespace core

} // namespace maki
