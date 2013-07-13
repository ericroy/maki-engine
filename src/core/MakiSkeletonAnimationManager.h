#pragma once
#include "core/core_stdafx.h"
#include "core/MakiManager.h"
#include "core/MakiSkeletonAnimation.h"

namespace Maki
{
	namespace Core
	{

		class SkeletonAnimationManager;
		

		class SkeletonAnimationManager : public Manager<SkeletonAnimation, SkeletonAnimationManager>
		{
		public:
			static const int32 DEFAULT_SIZE = 32;

		public:
			SkeletonAnimationManager(uint32 size = DEFAULT_SIZE);	
			virtual ~SkeletonAnimationManager();
			Handle Load(Rid rid);
			void ReloadAssets();
			bool ReloadAsset(Rid rid);
		};

	} // namespace Core

} // namespace Maki