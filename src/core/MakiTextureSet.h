#pragma once
#include "core/MakiTypes.h"
#include "core/MakiTexture.h"
#include "core/MakiResourcePool.h"

namespace maki {
	namespace core {

		class texture_set_t {
			MAKI_NO_COPY(texture_set_t);
		public:
			static const int32_t max_textures_per_set = 4;

		public:
			texture_set_t() = default;
			texture_set_t(texture_set_t &&other);
			bool load(uint8_t count, rid_t *texture_rids);
			inline bool operator==(const texture_set_t &other) const;
			
		public:
			uint8_t texture_count = 0;
			ref_t<texture_t> textures[max_textures_per_set];
			rid_t texture_rids[max_textures_per_set];
		};


		bool texture_set_t::operator==(const texture_set_t &other) const {
			if(texture_count != other.texture_count) {
				return false;
			for(uint32_t i = 0; i < texture_count; i++) {
				if(texture_rids[i] != other.texture_rids[i] || texture_rids[i] == RID_NONE)
					return false;
			}
			return true;
		}

	} // namespace core
} // namespace maki
