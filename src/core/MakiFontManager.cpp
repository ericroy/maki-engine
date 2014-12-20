#include "core/core_stdafx.h"
#include "core/MakiFontManager.h"

namespace maki
{
	namespace core
	{

		inline bool font_manager_t::predicate_t::operator()(const font_t *font) const
		{
			return font->rid_ == font_rid && font->pixel_size == pixel_size && font->shader_program_rid == shader_program_rid;
		}

		font_manager_t::font_manager_t(uint32 size)
			: manager_t<font_t, font_manager_t>(size, "font_manager_t")
		{
		}
	
		font_manager_t::~font_manager_t()
		{
		}

		handle_t font_manager_t::load(rid_t shader_program_rid, rid_t font_rid, uint32 pixel_size)
		{
		
			predicate_t p;
			p.font_rid_ = font_rid;
			p.shader_program_rid_ = shader_program_rid;
			p.pixel_size_ = pixel_size;
			handle_t handle = resPool->Match(p);
		
			if(handle != HANDLE_NONE) {
				return handle;
			}

			handle = resPool->alloc();
			assert(handle != HANDLE_NONE && "font_t pool depleted");
			font_t *font = resPool->get(handle);
			new(font) font_t();
			if(!font->load(shader_program_rid, font_rid, pixel_size)) {
				resPool->free(handle);
				return HANDLE_NONE;
			}
			return handle;
		}

	} // namespace core

} // namespace maki
