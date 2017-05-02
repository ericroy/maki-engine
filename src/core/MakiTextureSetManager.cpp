#include "core/MakiTextureSetManager.h"
#include "core/MakiDrawCommand.h"

namespace maki {
	namespace core {

		texture_set_manager_t::texture_set_manager_t(uint32_t capacity) : res_pool_(capacity, "texture_set_manager_t") {
			MAKI_ASSERT(capacity <= (1 << draw_command_t::bits_per_texture_set) && "assign more bits to texture set in draw command bitfield");
		}

		ref_t<texture_set_t> texture_set_manager_t::create() {
			return res_pool_.alloc();
		}

		ref_t<texture_set_t> texture_set_manager_t::create(uint8_t count, ref_t<texture_t> *textures) {
			auto tex = res_pool_.alloc();
			return tex->init(count, ref_t<texture_t> *textures) ? tex : nullptr;
		}

		ref_t<texture_set_t> texture_set_manager_t::get(uint8_t count, rid_t *rids) {
			return res_pool_.find([rids, count](const texture_set_t &ts) {
				return ts.equals(count, rids);
			});
		}

		ref_t<texture_set_t> texture_set_manager_t::load(uint8_t count, rid_t *rids) {
			auto ts = res_pool_.alloc();
			return ts->load(count, rids) ? ts : nullptr;
		}

		ref_t<texture_set_t> texture_set_manager_t::get_or_load(uint8_t count, rid_t *rids) {
			auto ts = get(count, rids);
			return ts ? ts : load(count, rids);
		}


	} // namespace core
} // namespace maki
