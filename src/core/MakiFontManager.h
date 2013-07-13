#pragma once
#include "core/core_stdafx.h"
#include "core/MakiFont.h"
#include "core/MakiManager.h"

namespace Maki
{
	namespace Core
	{
		class FontManager;
		

		class FontManager : public Manager<Font, FontManager>
		{
		private:
			struct Predicate : std::unary_function<const Font *, bool>
			{
				inline bool operator()(const Font *font) const;
				Rid shaderProgramRid;
				Rid fontRid;
				uint32 pixelSize;
			};
		
		public:
			static const int32 DEFAULT_SIZE = 8;

		public:
			FontManager(uint32 size = DEFAULT_SIZE);	
			virtual ~FontManager();
			Handle Load(Rid shaderProgramRid, Rid fontRid, uint32 pixelSize);
		};

	} // namespace Core

} // namespace Maki
