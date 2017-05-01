#pragma once
#include "core/MakiMacros.h"
#include "core/MakiFont.h"
#include "core/MakiManager.h"

namespace maki {
	namespace core {

		class font_manager_t : public manager_t<font_t, font_manager_t> {
			MAKI_NO_COPY(font_manager_t);

		private:
			struct predicate_t : std::unary_function<const font_t *, bool> {
				inline bool operator()(const font_t *font) const;
				rid_t shader_program_rid = RID_NONE;
				rid_t font_rid = RID_NONE;
				uint32_t pixel_size = 0;
			};
		
		public:
			font_manager_t(uint64_t capacity);	
			virtual ~font_manager_t() = default;
			handle_t load(rid_t shader_program_rid, rid_t font_rid, uint32_t pixel_size);
		};

	} // namespace core
} // namespace maki
