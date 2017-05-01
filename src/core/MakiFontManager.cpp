#include "core/MakiFontManager.h"

namespace maki {
	namespace core {

		inline bool font_manager_t::predicate_t::operator()(const font_t *font) const {
			return font->rid == font_rid && font->pixel_size == pixel_size && font->shader_program_rid == shader_program_rid;
		}

		font_manager_t::font_manager_t(uint64_t capacity)
			: manager_t<font_t, font_manager_t>(capacity, "font_manager_t") {
		}

		handle_t font_manager_t::load(rid_t shader_program_rid, rid_t font_rid, uint32_t pixel_size) {
			predicate_t p;
			p.font_rid = font_rid;
			p.shader_program_rid = shader_program_rid;
			p.pixel_size = pixel_size;
			handle_t handle = res_pool_->match(p);
		
			if(handle != HANDLE_NONE)
				return handle;

			handle = res_pool_->alloc();
			assert(handle != HANDLE_NONE && "font_t pool depleted");
			font_t *font = res_pool_->get(handle);
			new(font) font_t();
			if(!font->load(shader_program_rid, font_rid, pixel_size)) {
				res_pool_->free(handle);
				return HANDLE_NONE;
			}
			return handle;
		}

	} // namespace core
} // namespace maki
