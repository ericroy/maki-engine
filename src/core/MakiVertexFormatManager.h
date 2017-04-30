#pragma once
#include "core/core_stdafx.h"
#include "core/MakiVertexFormat.h"
#include "core/MakiManager.h"

namespace maki
{
	namespace core
	{
		class vertex_format_manager_t;
		

		class vertex_format_manager_t : public manager_t<vertex_format_t, vertex_format_manager_t>
		{
		public:
			static const int32_t default_size_ = 8;

		public:
			inline handle_t find_or_add(const vertex_format_t &vf)
			{
				handle_t vertex_format = find(vf);
				if(vertex_format == HANDLE_NONE) {
					return add(vf);
				}
				return vertex_format;
			}

		public:
			vertex_format_manager_t(uint32_t size = default_size_);	
			virtual ~vertex_format_manager_t();
		};

	} // namespace core

} // namespace maki
