#pragma once
#include "core/core_stdafx.h"
#include "core/MakiResource.h"
#include "core/MakiSkeleton.h"

namespace Maki
{
	namespace Core
	{
		class Skeleton;

		class MAKI_CORE_API SkeletonAnimation : public Resource
		{
		public:
			struct MAKI_CORE_API State
			{
			public:
				State(uint32 size);
				State();
				void SetSize(uint32 size);

			public:
				Array<uint32> currentKeyFrames;
				float currentFrame;
			};

		private:
			struct MAKI_CORE_API KeyFrame : public Skeleton::Joint
			{
				uint32 frame;
			};

		public:
			static float debugRateCoeff;

		public:
			SkeletonAnimation();
			~SkeletonAnimation();
			bool Load(Rid rid);
			void AdvanceState(float timeDelta, float rateCoeff, bool loop, State &state, Array<Skeleton::Joint> &pose);
			inline uint32 GetBoneCount() const;

		public:
			float frameRate;
			uint32 frameCount;

			// Indexed as data[boneIndex][keyframeIndex]
			Array< Array<KeyFrame> > data;
		};



		uint32 SkeletonAnimation::GetBoneCount() const
		{
			return data.count;
		}

	} // namespace Core

} // namespace Maki