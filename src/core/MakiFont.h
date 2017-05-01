#pragma once
#include "dependencies/stb_truetype.h"
#include "core/MakiResource.h"


namespace maki {
	namespace core {

		class mesh_t;

		class font_t : public resource_t
		{
			friend class font_manager_t;
	
		private:
			static const int32_t min_char_code_ = 32;
			static const int32_t max_char_code_ = 126;
			static const int32_t char_code_count_ = max_char_code_ - min_char_code_ + 1;

		public:
			font_t();
			font_t(font_t &&) { assert(false && "font_t move construction not allowed"); }
			font_t(const font_t &) { assert(false && "font_t copy construction not allowed"); }
			virtual ~font_t();
			bool operator==(const font_t &other) const;
			bool load(rid_t shader_program_rid, rid_t font_rid, uint32_t pixel_size);
			void render_as_mesh(const char *s, mesh_t *m);

		public:
			handle_t material_;
			rid_t shader_program_rid_;
			uint32_t pixel_size_;
	
		private:
			uint32_t texture_width_;
			uint32_t texture_height_;
			stbtt_bakedchar baked_chars_[char_code_count_];
		};

	} // namespace core
} // namespace maki
