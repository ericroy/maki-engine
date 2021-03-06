#pragma once
#include "core/core_stdafx.h"

namespace Maki
{
	namespace Core
	{

		class TextureSet
		{
		public:
			static const int32 MAX_TEXTURES_PER_SET = 4;

		public:
			TextureSet();
			TextureSet(const MoveToken<TextureSet> &other);
			TextureSet(const TextureSet &) { assert(false && "TextureSet copy construction not allowed"); }
			~TextureSet();
			bool Load(uint8 count, Rid *textureRids);
			inline bool operator==(const TextureSet &other) const;
			
		public:
			uint8 textureCount;
			Handle textures[MAX_TEXTURES_PER_SET];
			Rid textureRids[MAX_TEXTURES_PER_SET];
		};


		bool TextureSet::operator==(const TextureSet &other) const
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

	} // namespace Core

} // namespace Maki