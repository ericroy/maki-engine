#pragma once
#include "core/core_stdafx.h"
#include "core/MakiResource.h"
#include "core/MakiSkeleton.h"

namespace Maki
{
	namespace Core
	{
		class Skeleton;

		MAKI_CORE_API_EXTERN template class MAKI_CORE_API Array<uint32>;

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
			SkeletonAnimation(const MoveToken<SkeletonAnimation> &) { assert(false && "SkeletonAnimation move construction not allowed"); }
			SkeletonAnimation(const SkeletonAnimation &) { assert(false && "SkeletonAnimation copy construction not allowed"); }
			~SkeletonAnimation();
			bool Load(Rid rid);
			void AdvanceState(float timeDelta, float rateCoeff, bool loop, State &state, Array<Skeleton::Joint> &pose);
			inline uint32 GetBoneCount() const;

		public:
			float frameRate;
			uint32 frameCount;

			// Indexed as data[boneIndex][keyframeIndex]
#pragma warning(disable:4251)
			Array< Array<KeyFrame> > data;
#pragma warning(default:4251)
		};

		uint32 SkeletonAnimation::GetBoneCount() const
		{
			return data.count;
		}

		MAKI_CORE_API_EXTERN template class MAKI_CORE_API Array<SkeletonAnimation::KeyFrame>;
		MAKI_CORE_API_EXTERN template class MAKI_CORE_API Array< Array<SkeletonAnimation::KeyFrame> >;

	} // namespace Core

} // namespace Maki