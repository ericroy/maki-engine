#include "core/MakiTextureSet.h"
#include "core/MakiEngine.h"
#include "core/MakiCoreManagers.h"
#include "core/MakiTextureManager.h"


namespace maki {
	namespace core {
	
		texture_set_t::texture_set_t(texture_set_t &&other) {
			for(uint8_t i = 0; i < other.count_; i++) {
				textures_[i] = move(other.textures_[i]);
				texture_rids_[i] = other.texture_rids_[i];
			}
			count_ = other.count_;
			other.count_ = 0;
		}

		bool texture_set_t::equals(uint8_t count, const rid_t *rids) const {
			if(count_ != count)
				return false;
			for(uint8_t i = 0; i < count_; i++) {
				if(texture_rids_[i] != rids[i] || texture_rids_[i] == RID_NONE)
					return false;
			}
			return true;
		}

		bool texture_set_t::load(uint8_t count, rid_t *texture_rids) {
			MAKI_ASSERT(count_ == 0 && "Texture set already initialized");
			auto res = core_managers_t::get();
			for(uint8_t i = 0; i < count; i++) {
				texture_rids_[i] = texture_rids[i];
				textures_[i] = res->texture_manager->get_or_load(texture_rids_[i]);
				if (!textures_[i])
					return false;
			}
			count_ = count;
			return true;
		}

		bool texture_set_t::init(uint8_t count, ref_t<texture_t> *textures) {
			MAKI_ASSERT(count_ == 0 && "Texture set already initialized");
			for(uint8_t i = 0; i < count; i++) {
				texture_rids_[i] = textures[i]->rid();
				textures_[i] = textures[i];
				if (!textures_[i])
					return false;
			}
			count_ = count;
			return true;
		}

	} // namespace core
} // namespace maki
