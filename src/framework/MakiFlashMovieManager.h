#pragma once
#include "framework/framework_stdafx.h"
#include "core/MakiManager.h"
#include "framework/MakiFlashMovie.h"

namespace Maki
{
	namespace Framework
	{

		class FlashMovieManager : public Core::Manager<FlashMovie, FlashMovieManager>
		{
		public:
			static const int32 DEFAULT_SIZE = 32;

		public:
			FlashMovieManager(uint32 size = DEFAULT_SIZE);	
			virtual ~FlashMovieManager();
			Handle Load(Rid rid);
			void ReloadAssets();
			bool ReloadAsset(Rid rid);
		};

	} // namespace Core

} // namespace Maki