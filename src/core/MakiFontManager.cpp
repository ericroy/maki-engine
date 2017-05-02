#include "core/MakiFontManager.h"

namespace maki {
	namespace core {

		font_manager_t::font_manager_t(uint32_t capacity) {
			res_pool_.reset(new resouce_pool_t<font_t>(capacity, "font_manager_t"));
		}

		ref_t<font_t> font_manager_t::get(rid_t rid, rid_t shader_program_rid, uint32_t pixel_size) {
			return res_pool_->find([rid, shader_program_rid, pixel_size](const font_t &f) {
				return rid == f.rid() && pixel_size == f.pixel_size && shader_program_rid == f.shader_program_rid;
			});
		}

		ref_t<font_t> font_manager_t::load(rid_t rid, rid_t shader_program_rid, uint32_t pixel_size) {
			auto font = res_pool_->alloc();
			return font->load(rid, shader_program_rid, pixel_size) ? font : nullptr;
		}

		ref_t<font_t> font_manager_t::get_or_load(rid_t rid, rid_t shader_program_rid, uint32_t pixel_size) {
			auto font = get(rid, shader_program_rid, pixel_size);
			return font ? font : load(rid, shader_program_rid, pixel_size);
		}

	} // namespace core
} // namespace maki
