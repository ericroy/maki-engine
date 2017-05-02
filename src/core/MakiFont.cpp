#include "core/MakiFont.h"
#include "core/MakiEngine.h"
#include "core/MakiCoreManagers.h"
#include "core/MakiRenderer.h"
#include "core/MakiAssetLibrary.h"
#include "core/MakiMesh.h"
#include "core/MakiTextureSet.h"
#include "core/MakiTextureSetManager.h"
#include "core/MakiConsole.h"

namespace maki {
	namespace core {

		inline uint32_t next_power_of_two(uint32_t v) {
			v--;
			v |= v >> 1;
			v |= v >> 2;
			v |= v >> 4;
			v |= v >> 8;
			v |= v >> 16;
			v++;
			return v;
		}

		bool font_t::load(rid_t font_rid, rid_t shader_program_rid, uint32_t pixel_size) {
			auto *res = core_managers_t::get();
			auto *eng = engine_t::get();

			auto font_data = eng->assets->alloc_read(font_rid);
			if(!font_data) {
				console_t::error("Failed to load font <rid %u>", font_rid);
				return false;
			}

			texture_width_ = 512;
			texture_height_ = next_power_of_two(pixel_size);
			array_t<char> pixels(texture_width_ * texture_height_);
			
			int ret;
			while((ret = stbtt_BakeFontBitmap((const uint8_t *)font_data.data(), 0, (float)pixel_size, (unsigned char *)pixels.data(), texture_width_, texture_height_, min_char_code_, char_code_count_, baked_chars_)) <= 0) {
				texture_height_ *= 2;
				pixels.set_length(texture_width_ * texture_height_);
			}

			auto glyph_atlas = res->texture_manager->create(texture_type_regular, texture_width_, texture_height_, 1);
			eng->renderer->write_to_texture(glyph_atlas.ptr(), 0, 0, 0, 0, texture_width_, texture_height_, texture_width_, 1, pixels.data());
		
			material_ = res->material_manager->create();
			material_->set_shader_program(move(res->shader_program_manager->get_or_load(shader_program_rid)));

			ref_t<texture_t> textures[] = { glyph_atlas };
			auto ts = res->texture_set_manager->create(1, textures);
			material_->set_texture_set(move(ts));

			shader_program_rid_ = shader_program_rid;
			pixel_size_ = pixel_size;
			rid_ = font_rid;
			return true;
		}

		void font_t::render_as_mesh(const char *s, mesh_t *m) {
			m->set_vertex_attributes(attribute_flag_tex_coord);
			m->set_index_attributes(3, 2);
			m->set_mesh_flag(mesh_t::mesh_flag_has_translucency);

			float pen_x = 0.0f;
			float pen_y = 0.0f;
		
			struct vertex_t {
				float pos[3];
				float uv[2];
			};

			stbtt_aligned_quad q;

			const uint32_t len = strlen(s);
			for(uint32_t i = 0; i < len; i++) {
				if(s[i] < min_char_code_ || s[i] > max_char_code_) {
					continue;
				}
			
				stbtt_GetBakedQuad(baked_chars_, texture_width_, texture_height_, s[i] - min_char_code_, &pen_x, &pen_y, &q, 1);

				vertex_t v[4] = {
					{q.x0, q.y0, 0, q.s0, q.t0},
					{q.x0, q.y1, 0, q.s0, q.t1},
					{q.x1, q.y1, 0, q.s1, q.t1},
					{q.x1, q.y0, 0, q.s1, q.t0},
				};
				m->push_vertex_data(sizeof(v), (char *)v);

				uint16_t base = i*4;
				uint16_t f[6] = {base, (uint16_t)(base+1), (uint16_t)(base+2), base, (uint16_t)(base+2), (uint16_t)(base+3)};
				m->push_index_data(sizeof(f), (char *)f);
			}

			m->upload();
		}


	} // namespace core
} // namespace maki
