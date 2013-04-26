#pragma once
#include "core/core_stdafx.h"
#include "core/MakiResource.h"
#include "core/MakiSkeleton.h"

namespace Maki
{
	class Skeleton;

	class SkeletonAnimation : public Resource
	{
	public:
		static float debugRateCoeff;

	public:
		struct State
		{
			State(uint32 size) : currentFrame(0.0f) { SetSize(size); }
			State() : currentFrame(0.0f) {}

			inline void SetSize(uint32 size)
			{
				currentKeyFrames.SetSize(size);
				currentKeyFrames.Zero();
			}

			Array<uint32> currentKeyFrames;
			float currentFrame;
		};

	private:
		struct KeyFrame : public Skeleton::Joint
		{
			uint32 frame;
		};

	public:
		SkeletonAnimation();
		~SkeletonAnimation();
		bool Load(Rid rid);
		void AdvanceState(float timeDelta, float rateCoeff, bool loop, State &state, Array<Skeleton::Joint> &pose);
		inline uint32 GetBoneCount() const { return data.count; }

	public:
		float frameRate;
		uint32 frameCount;

		// Indexed as data[boneIndex][keyframeIndex]
		Array< Array<KeyFrame> > data;
	};

} // namespace Maki