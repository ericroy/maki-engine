#include "core/MakiTexture.h"
#include "core/MakiEngine.h"
#include "core/MakiRenderer.h"

namespace maki {
	namespace core {

		texture_t::~texture_t() {
			engine_t::get()->renderer->delete_texture(this);
		}

	
	} // namespace core
} // namespace maki
