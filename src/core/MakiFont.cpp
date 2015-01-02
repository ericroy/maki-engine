#include "core/core_stdafx.h"
#include "core/MakiFont.h"
#include "core/MakiEngine.h"
#include "core/MakiCoreManagers.h"
#include "core/MakiRenderer.h"
#include "core/MakiAssetLibrary.h"
#include "core/MakiMesh.h"
#include "core/MakiTextureSet.h"
#include "core/MakiTextureSetManager.h"

namespace maki
{
	namespace core
	{

		inline uint32 next_power_of_two(uint32 v)
		{
			v--;
			v |= v >> 1;
			v |= v >> 2;
			v |= v >> 4;
			v |= v >> 8;
			v |= v >> 16;
			v++;
			return v;
		}


		font_t::font_t()
			: resource_t(), material_(HANDLE_NONE), pixel_size_(0), texture_width_(0), texture_height_(0)
		{
		}

		font_t::~font_t()
		{
			material_manager_t::free(material_);
		}

		bool font_t::operator==(const font_t &other) const
		{
			return rid_ == other.rid_ && pixel_size_ == other.pixel_size_;
		}

		bool font_t::load(rid_t shader_program_rid, rid_t font_rid, uint32 pixel_size)
		{
			core_managers_t *res = core_managers_t::get();
			engine_t *eng = engine_t::get();

			char *font_data = eng->assets_->alloc_read(font_rid);
			if(font_data == nullptr) {
				console_t::error("Failed to load font <rid %d>", font_rid);
				return false;
			}

			texture_width_ = 512;
			texture_height_ = next_power_of_two(pixel_size);
			uint8 *pixels = (uint8 *)allocator_t::realloc(nullptr, texture_width_ * texture_height_);

			int ret;
			while((ret = stbtt_BakeFontBitmap((const uint8 *)font_data, 0, (float)pixel_size, pixels, texture_width_, texture_height_, min_char_code_, char_code_count_, baked_chars_)) <= 0) {
				texture_height_ *= 2;
				pixels = (uint8 *)allocator_t::realloc(pixels, texture_width_ * texture_height_);
			}

			handle_t glyph_atlas = res->texture_manager_->alloc_texture(texture_t::texture_type_regular_, texture_width_, texture_height_, 1);
			texture_t *tex_ = texture_manager_t::get(glyph_atlas);
			eng->renderer_->write_to_texture(tex_, 0, 0, 0, 0, texture_width_, texture_height_, texture_width_, 1, (char *)pixels);
		
			allocator_t::free(pixels);
			MAKI_SAFE_FREE(font_data);

			material_t mat;
			mat.set_shader_program(shader_program_rid);

			texture_set_t ts;
			// The texture set will take ownership of the glyph_atlas handle_
			ts.textures_[ts.texture_count_++] = glyph_atlas;

			// The material will take ownership of the new texture set handle_
			mat.texture_set_ = res->texture_set_manager_->add(maki_move(ts));

			// And finally, we will accept ownerhip of the material handle_
			material_ = res->material_manager_->add(maki_move(mat));


			pixel_size_ = pixel_size;
			rid_ = font_rid;
			return true;
		}

		void font_t::render_as_mesh(const char *s, mesh_t *m)
		{
			m->set_vertex_attributes(vertex_format_t::attribute_flag_text_coord_);
			m->set_index_attributes(3, 2);
			m->set_mesh_flag(mesh_t::mesh_flag_has_translucency_);

			float pen_x = 0.0f;
			float pen_y = 0.0f;
		
			struct V {
				float pos[3];
				float uv[2];
			};

			stbtt_aligned_quad q;

			const uint32 len = strlen(s);
			for(uint32 i = 0; i < len; i++) {
				if(s[i] < min_char_code_ || s[i] > max_char_code_) {
					continue;
				}
			
				stbtt_GetBakedQuad(baked_chars_, texture_width_, texture_height_, s[i] - min_char_code_, &pen_x, &pen_y, &q, 1);

				V v[4] = {
					{q.x0, q.y0, 0, q.s0, q.t0},
					{q.x0, q.y1, 0, q.s0, q.t1},
					{q.x1, q.y1, 0, q.s1, q.t1},
					{q.x1, q.y0, 0, q.s1, q.t0},
				};
				m->push_vertex_data(sizeof(v), (char *)v);

				uint16 base = i*4;
				uint16 f[6] = {base, (uint16)(base+1), (uint16)(base+2), base, (uint16)(base+2), (uint16)(base+3)};
				m->push_index_data(sizeof(f), (char *)f);
			}

			m->upload();
		}


	} // namespace core

} // namespace maki
