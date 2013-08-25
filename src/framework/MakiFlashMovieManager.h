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
		private:
			struct Predicate : public std::unary_function<const FlashMovie *, bool>
			{
				inline bool operator()(const FlashMovie *mov) const;
				Rid movieRid;
				Rid materialRid;
			};

		public:
			static const int32 DEFAULT_SIZE = 32;

		public:
			FlashMovieManager(uint32 size = DEFAULT_SIZE);	
			virtual ~FlashMovieManager();
			Handle Load(Rid movieRid, Rid materialRid);
			void ReloadAssets();
			bool ReloadAsset(Rid rid);
		};

		inline bool FlashMovieManager::Predicate::operator()(const FlashMovie *mov) const
		{
			return mov->rid == movieRid && mov->materialRid == materialRid;
		}

	} // namespace Core

} // namespace Maki