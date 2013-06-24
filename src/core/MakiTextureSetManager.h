#pragma once
#include "core/core_stdafx.h"
#include "core/MakiTextureSet.h"
#include "core/MakiManager.h"

namespace Maki
{
	namespace Core
	{

		class TextureSetManager : public Manager<TextureSet, TextureSetManager>
		{
		private:
			struct Predicate : public std::unary_function<const TextureSet *, bool>
			{
				inline bool operator()(const TextureSet *ts) const;
				uint8 count;
				Rid *textureRids;
			};

		public:
			static const int32 DEFAULT_SIZE = 128;

		public:
			TextureSetManager(uint32 size = DEFAULT_SIZE);	
			virtual ~TextureSetManager();	
			Handle Load(uint8 count, Rid *textureRids);
		};

	} // namespace Core

} // namespace Maki
