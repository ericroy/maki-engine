#pragma once
#include "core/core_stdafx.h"

namespace maki
{
	namespace core
	{

		class texture_set_t
		{
		public:
			static const int32 MAX_TEXTURES_PER_SET = 4;

		public:
			texture_set_t();
			texture_set_t(const move_token_t<texture_set_t> &other);
			texture_set_t(const texture_set_t &) { assert(false && "texture_set_t copy construction not allowed"); }
			~texture_set_t();
			bool load(uint8 count, rid_t *textureRids);
			inline bool operator==(const texture_set_t &other) const;
			
		public:
			uint8 textureCount;
			handle_t textures[MAX_TEXTURES_PER_SET];
			rid_t textureRids[MAX_TEXTURES_PER_SET];
		};


		bool texture_set_t::operator==(const texture_set_t &other) const
		{
			if(textureCount != other.textureCount) {
				return false;
			}
			for(uint32 i = 0; i < textureCount; i++) {
				if(textureRids[i] != other.textureRids[i] || textureRids[i] == RID_NONE) {
					return false;
				}
			}
			return true;
		}

	} // namespace core

} // namespace maki
