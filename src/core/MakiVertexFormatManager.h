#pragma once
#include "core/MakiMacros.h"
#include "core/MakiTypes.h"
#include "core/MakiVertexFormat.h"
#include "core/MakiManager.h"

namespace maki {
	namespace core {

		class vertex_format_manager_t : public manager_t<vertex_format_t, vertex_format_manager_t> {
			MAKI_NO_COPY(vertex_format_manager_t);

		public:
			vertex_format_manager_t(uint64_t capacity);	
			virtual ~vertex_format_manager_t() = default;

			inline handle_t find_or_add(const vertex_format_t &vf) {
				handle_t vertex_format = find(vf);
				if(vertex_format == HANDLE_NONE)
					return add(vf);
				return vertex_format;
			}
		};

	} // namespace core
} // namespace maki
