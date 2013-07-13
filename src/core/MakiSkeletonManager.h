#pragma once
#include "core/core_stdafx.h"
#include "core/MakiManager.h"
#include "core/MakiSkeleton.h"

namespace Maki
{
	namespace Core
	{

		class SkeletonManager;
		MAKI_CORE_API_EXTERN template class MAKI_CORE_API Manager<Skeleton, SkeletonManager>;

		class MAKI_CORE_API SkeletonManager : public Manager<Skeleton, SkeletonManager>
		{
		public:
			static const int32 DEFAULT_SIZE = 8;

		public:
			SkeletonManager(uint32 size = DEFAULT_SIZE);	
			virtual ~SkeletonManager();
			Handle Load(Rid rid);
			void ReloadAssets();
			bool ReloadAsset(Rid rid);
		};

	} // namespace Core

} // namespace Maki