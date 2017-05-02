#pragma once
#include "dependencies/stb_truetype.h"
#include "core/MakiTypes.h"
#include "core/MakiMaterial.h"
#include "core/MakiResourcePool.h"


namespace maki {
	namespace core {

		class mesh_t;

		class font_t {
			MAKI_NO_COPY(font_t);
			friend class font_manager_t;
	
		private:
			static const int32_t min_char_code_ = 32;
			static const int32_t max_char_code_ = 126;
			static const int32_t char_code_count_ = max_char_code_ - min_char_code_ + 1;

		public:
			font_t() = default;
			bool load(rid_t font_rid, rid_t shader_program_rid, uint32_t pixel_size);
			void render_as_mesh(const char *s, mesh_t *m);

		public:
			rid_t rid = RID_NONE;
			ref_t<material_t> material;
			rid_t shader_program_rid;
			uint32_t pixel_size = 0;
	
		private:
			uint32_t texture_width_ = 0;
			uint32_t texture_height_ = 0;
			stbtt_bakedchar baked_chars_[char_code_count_] = {};
		};

	} // namespace core
} // namespace maki
