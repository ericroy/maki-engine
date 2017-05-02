#pragma once
#include "core/MakiTypes.h"
#include "core/MakiVertexFormat.h"
#include "core/MakiResourcePool.h"

namespace maki {
	namespace core {

		class vertex_format_manager_t {
			MAKI_NO_COPY(vertex_format_manager_t);
		public:
			vertex_format_manager_t(uint32_t capacity);	
			virtual ~vertex_format_manager_t() = default;
			inline handle_t get_or_add(const vertex_format_t &vf) {
				return res_pool_->find([&vf](const vertex_format_t &vf2) {
					return vf == vf2;
				});
			}
		private:
			unique_ptr<resource_pool_t<vertex_format_t>> res_pool_;
		};

	} // namespace core
} // namespace maki
