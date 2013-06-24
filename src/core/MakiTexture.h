#pragma once
#include "core/core_stdafx.h"
#include "core/MakiResource.h"

namespace Maki
{
	namespace Core
	{

		class Texture : public Resource
		{
		public:
			enum TextureType
			{
				TextureType_Regular = 0,
				TextureType_DepthStencil,
				TextureType_RenderTarget,
			};

		public:
			Texture();
			~Texture();

		private:
			Texture(const Texture &) {}

		public:
			TextureType type;
			uint32 width;
			uint32 height;
			uint32 handle;
		};

	} // namespace Core

} // namespace Maki