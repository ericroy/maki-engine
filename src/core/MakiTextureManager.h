#pragma once
#include "core/core_stdafx.h"
#include "core/MakiTexture.h"
#include "core/MakiManager.h"

namespace Maki
{
	namespace Core
	{
		class TextureManager;
		MAKI_CORE_API_EXTERN template class MAKI_CORE_API Manager<Texture, TextureManager>;

		class MAKI_CORE_API TextureManager : public Manager<Texture, TextureManager>
		{
		public:
			static const int32 DEFAULT_SIZE = 64;

		public:
			TextureManager(uint32 size = DEFAULT_SIZE);	
			virtual ~TextureManager();
			Handle Load(Rid rid);
			Handle AllocTexture(Texture::TextureType type, uint32 width, uint32 height, uint8 channels);
			void ReloadAssets();
			bool ReloadAsset(Rid rid);

		private:
			bool LoadData(Texture *tex, Rid rid);
		};

	} // namespace Core

} // namespace Maki
