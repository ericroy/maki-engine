#pragma once
#include "core/core_stdafx.h"
#include "core/MakiResource.h"

namespace Maki
{
	namespace Core
	{

		class MAKI_CORE_API Texture : public Resource
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
			Texture(const MoveToken<Texture> &) { assert(false && "Texture move construction not allowed"); }
			Texture(const Texture &) { assert(false && "Texture copy construction not allowed"); }
			~Texture();

		public:
			TextureType type;
			uint32 width;
			uint32 height;
			uint32 handle;
		};

	} // namespace Core

} // namespace Maki