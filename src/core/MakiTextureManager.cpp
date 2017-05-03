#include "core/MakiTextureManager.h"
#include "core/MakiEngine.h"
#include "core/MakiRenderer.h"
#include "core/MakiConsole.h"

namespace maki {
	namespace core {

		texture_manager_t::texture_manager_t(uint32_t capacity) : res_pool_(capacity, "texture_manager_t") {
		}

		ref_t<texture_t> texture_manager_t::get(rid_t rid) {
			return res_pool_.find([rid](const texture_t &tex) { return rid == tex.rid(); });
		}

		ref_t<texture_t> texture_manager_t::load(rid_t rid) {
			auto tex = res_pool_.alloc();
			return tex->load(rid) ? tex : nullptr;
		}

		ref_t<texture_t> texture_manager_t::get_or_load(rid_t rid) {
			auto ref = get(rid);
			return ref ? ref : load(rid);
		}
	
		ref_t<texture_t> texture_manager_t::create(texture_type_t type, uint32_t width, uint32_t height, uint8_t channels) {
			auto tex = res_pool_.alloc();
			return tex->init(type, width, height, channels) ? tex : nullptr;
		}


	} // namespace core
} // namespace maki
