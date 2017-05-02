#pragma once
#include "core/MakiTypes.h"
#include "core/MakiTexture.h"
#include "core/MakiResourcePool.h"

namespace maki {
	namespace core {

		namespace {
			const int32_t max_textures_per_set = 4;
		}

		class texture_set_t {
			MAKI_NO_COPY(texture_set_t);
		public:
			texture_set_t() = default;
			texture_set_t(texture_set_t &&other);

			inline uint8_t count() const { return count_; }

			inline bool operator==(const texture_set_t &other) const {
				return equals(other.count_, other.texture_rids_);
			}

			inline const ref_t<texture_t> &get_texture(uint8_t index) const {
				MAKI_ASSERT(index < count_ && "Texture index out of range");
				return textures_[index];
			}

			inline rid_t get_texture_rid(uint8_t index) const {
				MAKI_ASSERT(index < count_ && "Texture rid index out of range");
				return texture_rids_[index];
			}
			
			bool equals(uint8_t count, const rid_t *rids) const;
			bool load(uint8_t count, rid_t *texture_rids);
			bool create(uint8_t count, ref_t<texture_t> *textures);

		private:
			uint8_t count_ = 0;
			ref_t<texture_t> textures_[max_textures_per_set];
			rid_t texture_rids_[max_textures_per_set];
		};

	} // namespace core
} // namespace maki
