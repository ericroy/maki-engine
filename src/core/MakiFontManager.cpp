#include "core/core_stdafx.h"
#include "core/MakiFontManager.h"

namespace maki
{
	namespace core
	{

		inline bool font_manager_t::predicate_t::operator()(const font_t *font) const
		{
			return font->rid_ == font_rid_ && font->pixel_size_ == pixel_size_ && font->shader_program_rid_ == shader_program_rid_;
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
			handle_t handle_ = res_pool_->match(p);
		
			if(handle_ != HANDLE_NONE) {
				return handle_;
			}

			handle_ = res_pool_->alloc();
			assert(handle_ != HANDLE_NONE && "font_t pool depleted");
			font_t *font = res_pool_->get(handle_);
			new(font) font_t();
			if(!font->load(shader_program_rid, font_rid, pixel_size)) {
				res_pool_->free(handle_);
				return HANDLE_NONE;
			}
			return handle_;
		}

	} // namespace core

} // namespace maki
