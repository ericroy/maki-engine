#pragma once
#include "framework/framework_stdafx.h"

namespace Maki
{
	namespace Framework
	{

		class AnimationBlender
		{
		public:
			class Node
			{
			public:
				Node(float rate);
				virtual ~Node();
				virtual void AdvanceState(float dt, Array<Skeleton::Joint> &pose, float rate) = 0;

			public:
				uint32 boneCount;
				float rate;
			};

			class Anim : public Node
			{
			public:
				Anim(float rate, bool loop, HandleOrRid animId);
				virtual ~Anim();
				void AdvanceState(float dt, Array<Skeleton::Joint> &pose, float rate);
				void SetFrame(float frame);

			public:
				bool loop;
				SkeletonAnimation::State state;
				Handle animation;
			};

			class Blend : public Node
			{
			public:
				Blend(float rate, float balance, Node *first, Node *second);			
				virtual ~Blend();
				void AdvanceState(float dt, Array<Skeleton::Joint> &pose, float rate);
			
			public:
				Tween<float> balance;
				Node *first;
				Node *second;
				Array<Skeleton::Joint> tempPose;
			};
		
		public:
			AnimationBlender();
			~AnimationBlender();
			bool Load(Rid rid);
			void AdvanceState(float dt, Array<Skeleton::Joint> &pose, float rate);

		private:
			Node *LoadRecursive(Document::Node *n);

		public:
			Node *root;
			std::vector<Tween<float> *> tweens;
		};

	} // namespace Framework

} // namespace Maki