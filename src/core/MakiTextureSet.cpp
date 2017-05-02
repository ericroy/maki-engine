#include "core/MakiTextureSet.h"
#include "core/MakiEngine.h"
#include "core/MakiCoreManagers.h"
#include "core/MakiTextureManager.h"


namespace maki {
	namespace core {
	
		texture_set_t::texture_set_t(texture_set_t &&other) {
			for(uint8_t i = 0; i < max_textures_per_set; i++) {
				textures[i] = move(other.textures[i]);
				texture_rids[i] = other.texture_rids[i];
			}
			texture_count = other.texture_count;
			other.texture_count = 0;
		}

		bool texture_set_t::load(uint8_t count, rid_t *texture_rids) {
			MAKI_ASSERT(texture_count == 0 && "Cannot load into texture set, already had textures in it!");
			auto res = core_managers_t::get();
			for(uint8_t i = 0; i < count; i++) {
				this->texture_rids[i] = texture_rids[i];
				this->textures[i] = res->texture_manager->get_or_load(texture_rids[i]);
				if (!this->textures[i])
					return false;
			}
			texture_count = count;
			return true;
		}

	} // namespace core
} // namespace maki
