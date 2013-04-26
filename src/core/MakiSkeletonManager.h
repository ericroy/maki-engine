#pragma once
#include "core/core_stdafx.h"
#include "core/MakiManager.h"
#include "core/MakiSkeleton.h"

namespace Maki
{

	class SkeletonManager : public Manager<Skeleton, SkeletonManager>
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

} // namespace Maki