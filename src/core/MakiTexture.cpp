#include "core/core_stdafx.h"
#include "core/MakiTexture.h"
#include "core/MakiEngine.h"
#include "core/MakiRenderer.h"

namespace maki
{
	namespace core
	{

		texture_t::texture_t() : resource_t(), type(texture_type_regular_), width(0), height(0), handle(-1) {
		}
	
		texture_t::~texture_t() {
			engine_t::get()->renderer_->delete_texture(this);
		}

	
	} // namespace core

} // namespace maki
