#pragma once
#include "core/core_stdafx.h"
#include "dependencies/stb_truetype.h"
#include "core/MakiResource.h"


namespace Maki
{
	namespace Core
	{

		class Mesh;

		class MAKI_CORE_API Font : public Resource
		{
			friend class FontManager;
	
		private:
			static const int32 MIN_CHAR_CODE = 32;
			static const int32 MAX_CHAR_CODE = 126;
			static const int32 CHAR_CODE_COUNT = MAX_CHAR_CODE - MIN_CHAR_CODE + 1;

		public:
			Font();
			virtual ~Font();
			bool operator==(const Font &other) const;
			bool Load(Rid shaderProgramRid, Rid fontRid, uint32 pixelSize);
			void RenderAsMesh(const char *s, Mesh *m);

		public:
			Handle material;
			Rid shaderProgramRid;
			uint32 pixelSize;
	
		private:
			uint32 textureWidth;
			uint32 textureHeight;
			stbtt_bakedchar bakedChars[CHAR_CODE_COUNT];
		};

	} // namespace Core

} // namespace Maki