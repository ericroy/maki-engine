#include "core/MakiTextureSet.h"
#include "core/MakiEngine.h"
#include "core/MakiCoreManagers.h"
#include "core/MakiTextureManager.h"


namespace maki {
	namespace core {

		texture_set_t::texture_set_t() : texture_count(0) {
			for(uint8_t i = 0; i < max_textures_per_set; i++) {
				textures[i] = HANDLE_NONE;
				texture_rids[i] = RID_NONE;
			}
		}
	
		texture_set_t::texture_set_t(texture_set_t &&other) : texture_count(other.texture_count) {
			memcpy(textures, other.textures, sizeof(textures));
			memcpy(texture_rids, other.texture_rids, sizeof(texture_rids));

			for(uint8_t i = 0; i < max_textures_per_set; i++) {
				other.textures[i] = HANDLE_NONE;
				other.texture_rids[i] = RID_NONE;
			}
			other.texture_count = 0;
		}

		texture_set_t::~texture_set_t() {
			texture_manager_t::free(texture_count, textures);
		}

		bool texture_set_t::load(uint8_t count, rid_t *texture_rids_) {
			auto res = core_managers_t::get();

			texture_count = count;
			for(uint8_t i = 0; i < count; i++) {
				this->texture_rids[i] = texture_rids_[i];
				this->textures[i] = res->texture_manager_->load(texture_rids_[i]);
				if(this->textures[i] == HANDLE_NONE) {
					texture_manager_t::free(texture_count, textures);
					return false;
				}
			}
			return true;
		}

	} // namespace core
} // namespace maki
