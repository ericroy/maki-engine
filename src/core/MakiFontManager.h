#pragma once
#include "core/MakiTypes.h"
#include "core/MakiFont.h"
#include "core/MakiResourcePool.h"

namespace maki {
	namespace core {

		class font_manager_t {
			MAKI_NO_COPY(font_manager_t);
		public:
			font_manager_t(uint32_t capacity);	
			virtual ~font_manager_t() = default;
			ref_t<font_t> get(rid_t rid, rid_t shader_program_rid, uint32_t pixel_size);
			ref_t<font_t> load(rid_t rid, rid_t shader_program_rid, uint32_t pixel_size);
			ref_t<font_t> get_or_load(rid_t rid, rid_t shader_program_rid, uint32_t pixel_size);
		private:
			unique_ptr<resource_pool_t<font_t>> res_pool_;
		};

	} // namespace core
} // namespace maki
