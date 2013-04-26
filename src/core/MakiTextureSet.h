#pragma once
#include "core/core_stdafx.h"

namespace Maki
{

	class TextureSet
	{
	public:
		static const int32 MAX_TEXTURES_PER_SET = 4;

	public:
		TextureSet();
		TextureSet(TextureSet &&other);
		~TextureSet();
		bool operator==(const TextureSet &other) const;
		bool Load(uint8 count, Rid *textureRids);

	private:
		// Prevent copy construction - only move semantics are allowed
		TextureSet(const TextureSet &other) {}

	public:
		uint8 textureCount;
		Handle textures[MAX_TEXTURES_PER_SET];
		Rid textureRids[MAX_TEXTURES_PER_SET];
	};

} // namespace Maki